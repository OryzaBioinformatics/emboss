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

import org.emboss.jemboss.parser.ParseAcd;
import java.awt.Dimension;
import javax.swing.JCheckBox;

public class AlignFormat
{

  /** EMBOSS align formats */
  private static String align_msf[] = 
            {"unknown", "multiple", "simple", 
             "fasta", "msf", "srs"};

  private static String align_pairwise[] =
            {"unknown", "multiple", "simple",
             "fasta", "msf", "srs",
             "pair", "markx0", "markx1",
             "markx2", "markx3", "markx10",
             "srspair", "score"};

  private myComboPopup cp;
  private String def;
  private boolean lpair;

  public AlignFormat(ParseAcd parseAcd, int nf) 
  {
    lpair = isPairWise(parseAcd,nf);
    cp = new myComboPopup(getAlignFormats(lpair));

    int np = parseAcd.getNumofParams(nf);

    for(int i=0;i<np;i++)
      if(parseAcd.getParameterAttribute(nf,i).equals("aformat"))
      {
        def = parseAcd.getParamValueStr(nf,i).toLowerCase();
        cp.setSelectedItem(def);
      }
 
    Dimension d = cp.getPreferredSize();
    d = new Dimension(150,(int)d.getHeight());
    
    cp.setMaximumSize(d);
    cp.setPreferredSize(d);
  }

  public myComboPopup getComboPopup()
  {
    return cp;
  }

  public String getDefaultFormat()
  {
    return def;
  }

  private String[] getAlignFormats(boolean lpair)
  {
    if(lpair)
      return align_pairwise;

    return align_msf;
  }

  public String getAlignFormat()
  {
    String report = " -aformat " + cp.getSelectedItem();
    return report;
  }

/**
*
* Method to determine whether this is a pairwise (returning
* true) or multiple sequence alignment format.
* @return true if a pairwise alignment
*
*/
  private boolean isPairWise(ParseAcd parseAcd, int nf)
  {
    for(int i=0; i<nf;i++)
      if(parseAcd.getParameterAttribute(i,0).equalsIgnoreCase("seqset"))
        return false;

    return true;
  }

/**
*
* @return String report of the available report formats
*
*/
  public String getToolTip()
  {
    String commonFormat =
        "unknown - synonym for simple format.\n"+
        "multiple - synonym for simple format.\n"+
        "simple - displays the sequence names and positions and\n"+
        "puts the markup line underneath the sequences.\n"+
        "fasta - Fasta sequence format with gaps\n"+
        "msf - MSF sequence format.\n"+
        "srs - shows the sequence ID name and position.\n";

    if(lpair)
      return commonFormat +
        "\npair - default format used with only 2 sequences.\n"+
        "markx0 - default output format for FASTA programs.\n"+
        "markx1 - alternative format for FASTA programs in which\n"+
        "identities are not marked. Conservative replacements are\n"+
        "denoted by \'x\' and non-conservative substitutions by \'X\'.\n"+
        "markx2 - alternative format for FASTA programs in which\n"+
        "residues in the second sequence are only shown if they\n"+
        "are different from the first.\n"+
        "markx3 - alternative format for FASTA programs in which\n"+
        "the sequences are displayed in FASTA format. These can\n"+
        "be used to build a primitive multiple alignment.\n"+
        "markx10 - alternative format for FASTA programs in which\n"+
        "the sequences are displayed in FASTA format and the\n"+
        "sequence length, alignment start and stop is given in\n"+
        "in lines starting with a semi colon after the title line\n"+
        "srspair - similar in style to pair format.\n"+
        "score - does not display the sequence alignment. Only\n"+
        "shows names of the sequences, length of the alignment and\n"+ 
        "the score in brackets.";

    return commonFormat;
  }

}


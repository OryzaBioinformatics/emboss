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

public class ReportFormat
{

  /** EMBOSS Report formats */
  private static String rpt[] = 
            {"embl", "genbank", "gff", "pir",
             "swiss", "listfile", "dbmotif",
             "diffseq", "excel", "feattable",
             "motif", "nametable", "regions",
             "seqtable", "simple", "srs",
             "table", "tagseq"};

  private myComboPopup cp;
  private JCheckBox raccshow = new JCheckBox();
  private JCheckBox rdesshow = new JCheckBox();
  private JCheckBox rusashow = new JCheckBox();
  private String def;

  public ReportFormat(ParseAcd parseAcd, int nf)
  {
    cp = new myComboPopup(getReportFormats());

    int np = parseAcd.getNumofParams(nf);

    for(int i=0;i<np;i++)
      if(parseAcd.getParameterAttribute(nf,i).equals("rformat"))
      {
        def = parseAcd.getParamValueStr(nf,i);
        cp.setSelectedItem(parseAcd.getParamValueStr(nf,i));
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

  public JCheckBox getAccCheckBox()
  {
    return raccshow;
  }

  public JCheckBox getDesCheckBox()
  {
    return rdesshow;
  }

  public JCheckBox getUsaDesCheckBox()
  {
    return rusashow;
  }

  
  private static String[] getReportFormats()
  {
    return rpt;
  }

  public String getReportFormat()
  {
    String report = " -rformat " + cp.getSelectedItem();
    if(raccshow.isSelected())
      report = report.concat(" -raccshow ");
    if(rdesshow.isSelected())
      report = report.concat(" -rdesshow ");
    if(rusashow.isSelected())
      report = report.concat(" -rusashow ");
    return report;
  }


/**
*
* @return String report of the available report formats
*
*/
  public static String getToolTip()
  {
    return "embl \t\t\t\t\t\t\t\t- EMBL feature table format.\n"+
           "genbank \t- Genbank feature table format.\n"+
           "gff \t\t\t\t\t\t\t\t\t\t\t\t- GFF feature table format.\n"+
           "pir \t\t\t\t\t\t\t\t\t\t\t\t- PIR feature table format.\n"+
           "swiss \t\t\t\t\t\t\t- SwissProt feature table format.\n"+
           "listfile \t\t\t\t\t\t- writes out a list file with start and end"+
           "points of the motifs given\nby '[start:end]' after the"+
           "sequence's full USA. This can be read by other EMBOSS\n"+
           "programs using '@' or 'list::' before the filename.\n"+
           "dbmotif \t\t\t\t- DbMotif format.\n"+
           "diffseq \t\t\t\t\t- Useful when reporting the results of two sequences\n"+
           "aligned, as in the program diffseq.\n"+
           "excel \t\t\t\t\t\t\t\t- TAB-delimited table format for spread-sheets\n"+
           "feattable \t\t- FeatTable format. It is an EMBL feature table\n"+
           "table using only the tags in the report definition.\n"+
           "motif \t\t\t\t\t\t\t\t- Motif format. Based on the original\n"+
           "output format of antigenic, helixturnhelix and sigcleave.\n"+ 
           "regions \t\t\t- Regions format.\n"+
           "seqtable \t\t- Simple table format that includes the feature sequence.\n"+
           "simple \t\t\t\t\t- SRS simple format.\n"+
           "srs \t\t\t\t\t\t\t\t\t\t\t- SRS format.\n"+
           "table \t\t\t\t\t\t\t- Table format.\n"+
           "tagseq \t\t\t\t-  Tagseq format. Features are marked up below the sequence.";
  }

}


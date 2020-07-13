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

import javax.swing.*;
import java.awt.FontMetrics;

public class LabelTextBox extends Box
{


/**
*
* @param String info text for the Component label
* @param String help text for the tool tip
*
*/
  public LabelTextBox(String sl, String tt)
  {
    super(BoxLayout.Y_AXIS);
 
    if(!sl.equals(""))
    {
      int stop;
      int width = 335;

      if(!tt.equals(""))
        tt = tt.replace('\\',' ');

      sl = sl.replace('\n',' ');
      String subLabel;
      JLabel l = new JLabel();     
      FontMetrics fm = l.getFontMetrics(SectionPanel.labfont);

      while(fm.stringWidth(sl) > width)
      {
        stop = sl.lastIndexOf(" ");
        subLabel = sl.substring(0,stop);
        while(fm.stringWidth(subLabel) > width)
        {
          stop = subLabel.lastIndexOf(" ");
          subLabel = subLabel.substring(0,stop);
        }

        l = new JLabel(" " + subLabel);
        add(l);
        sl = sl.substring(stop+1,sl.length());
        l.setFont(SectionPanel.labfont);
        l.setForeground(SectionPanel.labelColor);
        if(!tt.equals(""))
          l.setToolTipText(tt);
      }
      l = new JLabel(" " + sl);
      l.setFont(SectionPanel.labfont);
      l.setForeground(SectionPanel.labelColor);
      if(!tt.equals(""))
        l.setToolTipText(tt);
      add(l);
    }

  }


}


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
import javax.swing.*;

/**
*
* Draw sequence name
*
*/
public class SequenceNameJButton extends JToggleButton
{
  private Sequence seq;
  private int boundWidth = 6;
  private int fontSize = 14;
  private String nameLabel;
  private int ypad=0;
  private Font font = new Font("Monospaced",
                      Font.PLAIN, fontSize);

  public SequenceNameJButton(Sequence seq, int ypad)
  {
    super();
    this.seq = seq;
    this.ypad = ypad;
    if(seq.getID().equals(""))
      nameLabel = new String("");
    else
      nameLabel = new String(seq.getID()+"/"+
                             Integer.toString(seq.getLength()));

    setText(nameLabel);
    setBackground(Color.white);
//  setMaximumSize(getPreferredSize());
//  setMinimumSize(getPreferredSize());
    setHorizontalTextPosition(SwingConstants.RIGHT);
    setMargin(new Insets(0,0,0,0));
    setFont(font);
    setBorderPainted(false);
  }

  public void setFontSize(int size)
  {
    fontSize = size;
    this.font = new Font("Monospaced",
                          Font.PLAIN, fontSize);
    setMaximumSize(getPreferredSize());
    setMinimumSize(getPreferredSize());
    setFont(font);
  }

  public Font getFont()
  {
    return font;
  }

  public int getPanelWidth()
  {
    FontMetrics metrics = getFontMetrics(font);
    return metrics.stringWidth(nameLabel)+boundWidth;
  }

  public int getPanelHeight()
  {
    FontMetrics metrics = getFontMetrics(font);
    boundWidth = metrics.stringWidth("A");
    return metrics.stringWidth("A")+boundWidth;
  }

  public Dimension getPreferredSize()
  {
    return new Dimension(getPanelWidth(),
                         getPanelHeight()+ypad);
  }


}


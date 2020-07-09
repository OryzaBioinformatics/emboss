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

import javax.swing.JPanel;
import java.awt.*;


public class Separator extends JPanel
{
  private Color light;
  private Color dark;
  private Dimension d;

  public Separator(Dimension d)
  {
    super();
    this.d = d;
    setSize(d);
  }

  public void paint(Graphics g) 
  {
    Dimension size = getSize();
    light = getBackground().brighter().brighter();
//  dark  = getBackground().darker().darker();
    dark  = getBackground().darker();
    int length = d.width;
    int xpos = (size.width)/2 - length/2;
    int ypos = (size.height)/2;
    g.setColor(dark);
    g.drawLine(xpos,ypos,xpos+length,ypos);
    g.setColor(light);
    g.drawLine(xpos,ypos+1,xpos+length,ypos+1);
  }


}


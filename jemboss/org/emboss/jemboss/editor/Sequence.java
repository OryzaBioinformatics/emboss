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

/**
*
* Sequence object
*
*/
public class Sequence 
{

  private String name;   
  private String seq;
  private int length;
  private float wt;
  private String id;
  private boolean protein;

  public Sequence(String name, String seq, int length,
                  float wt)
  {
    this.name = name;
    this.seq = seq;
    this.length = length;
    this.wt = wt;
    int index = name.indexOf(" ");
    if(index>0)
      id = name.substring(0,index);
    else
      id = name;
  }

  public Sequence(String name, String seq, int length)
  {
    this(name,seq,length,1.0f);
  }

  public Sequence(String name, String seq)
  {
    this(name,seq,seq.length());
  }

  public Sequence(String seq)
  {
    this(new String(""),seq);
  }

  public Sequence(String seq, float wt)
  {
    this(new String(""),seq,seq.length(),wt);
  }

  public String getName()
  {
    return name;
  }

  public String getID()
  {
    return id;
  }

  public String getSequence()
  {
    return seq;
  }

  public int getLength()
  {
    return length;
  }

  public float getWeight()
  {
    return wt;
  }

  public String getResidue(int pos)
  {
    return seq.substring(pos,pos+1);
  }

  public void insertResidue(String s, int pos)
  {
    seq = seq.substring(0,pos)+s+
          seq.substring(pos);
    length++;
  }

  public void deleteResidue(int pos)
  {
    seq = seq.substring(0,pos)+
          seq.substring(pos+1);
    length--;
  }

  public void appentToSequence(String s)
  {
    seq = seq+s;
    length+=s.length();
  }

  public void reverseSequence()
  {
    char tmpChar[] = new char[length];
    for(int i = 0; i < length; i++)
      tmpChar[i] = seq.charAt(length-i-1);

    seq = new String(tmpChar);
  }

  public void reverseComplementSequence()
  {
    char tmpChar[] = new char[length];

    for(int i = 0; i < length; i++)
      tmpChar[i] = complement(seq.charAt(length-i-1));
    
    seq = new String(tmpChar);
  }
  
  public void setType(boolean protein)
  {
    this.protein = protein;
  }

  public boolean isProtein()
  {
    return protein;
  }

  public void complementSequence()
  {
    char tmpChar[] = new char[length];

    for(int i = 0; i < length; i++)
      tmpChar[i] = complement(seq.charAt(i));
   
    seq = new String(tmpChar);
  }

  private char complement(char c)
  {
    if(c == 't')
      return 'a';
    else if(c == 'T')
      return 'A';
    else if(c == 'a')
      return 't';
    else if(c == 'A')
      return 'T';
    else if(c == 'g')
      return 'c';
    else if(c == 'G')
      return 'C';
    else if(c == 'c')
      return 'g';
    else if(c == 'C')
      return 'G';
    return c;
  }

  public static void main(String args[])
  {
    new Sequence("Seq","ACTATACAG",9);
  }

}


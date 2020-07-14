/**
 * @author Ahmed Moustafa (ahmed at users.sourceforge.net)
 * $Id: Alignment.java,v 1.3 2003/09/24 09:07:40 timc Exp $
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
 */

package gnu.bioinformatics.jaligner.util;

import gnu.bioinformatics.jaligner.formats.Pair;

/**
 * A holder to the output of an alignment algorithm.
 * It holds the score, the two sequences aligned and the markup line.
 *
 * @author Ahmed Moustafa (ahmed at users.sourceforge.net)
 * @version $Revision: 1.3 $  
 */

public class Alignment {

	public static final char GAP				= '-';
	public static final char MARKUP_IDENTITY	= '|';
	public static final char MARKUP_SIMILARITY	= ':';
	public static final char MARKUP_GAP			= ' ';
	public static final char MARKUP_MISMATCH	= '.';

	private float score;
	private char[] sequence1;
	private String name1;
	private char[] sequence2;
	private String name2;
	private char[] markup;
	private int identity;
	private int similarity;
	private int gaps;
	private int offset1;
	private int offset2;
	
	/**
	 * Constructor for Alignment
	 */
	public Alignment( ) {
		super();
	}

	/**
	 * Returns the score of the alignment
	 * @return	score
	 */
	public float getScore() {
		return score;
	}

	/**
	 * Returns the 1st sequence aligned
	 * @return	1st squence
	 */
	public char[] getSequence1() {
		return sequence1;
	}

	/**
	 * Returns the 2nd sequence aligned
	 * @return	2nd sequence
	 */
	public char[] getSequence2() {
		return sequence2;
	}

	/**
	 * Returns the markup line
	 * @return	markup line
	 */
	public char[] getMarkup() {
		return markup;
	}

	/**
	 * Sets the alignment score
	 * @param score the score of the alignment
	 */
	public void setScore(float score) {
		this.score = score;
	}

	/**
	 * Sets the 1<sup>st</sup> sequence aligned
	 * @param sequence1	the 1st sequence aligned
	 */
	public void setSequence1(char[] sequence1) {
		this.sequence1 = sequence1;
	}

	/**
	 * Sets the 2<sup>nd</sup> sequence aligned
	 * @param sequence2	the 2nd sequence aligned
	 */
	public void setSequence2(char[] sequence2) {
		this.sequence2 = sequence2;
	}

	/**
	 * Sets the markup line
	 * @param markup the markup line of the alignment
	 */
	public void setMarkup(char[] markup) {
		this.markup = markup;
	}
	
	/**
	 * Sets the name for the 1<sup>st</sup> sequence
	 * @param name1	the name of the 1st sequence
	 */
	public void setName1(String name1) {
		this.name1 = name1;
	}

	/**
	 * Returns the name of the 1<sup>st</sup> sequence
	 * @return the name of the 1st sequence
	 */
	public String getName1( ) {
		return name1 == null ? "" : name1;
	}

	/**
	 * Sets the name for the 2<sup>nd</sup> sequence
	 * @param name2	the name of the 1st sequence
	 */
	public void setName2(String name2) {
		this.name2 = name2;
	}
	
	/**
	 * Returns the name of the 2<sup>nd</sup> sequence
	 * @return the name of the 2nd sequence
	 */
	public String getName2( ) {
		return name2 == null ? "" : name2;
	}

	/**
	 * Returns the number of gaps in the sequences
	 * @return the number of gaps
	 */
	public int getGaps() {
		return gaps;
	}

	/**
	 * Returns the number of identical residues or bases in the two sequences
	 * @return the identity
	 */
	public int getIdentity() {
		return identity;
	}

	/**
	 * Returns the number of similar residues or bases in the two sequences
	 * @return the similarity
	 */
	public int getSimilarity() {
		return similarity;
	}

	/**
	 * Sets the number of gaps in the sequences
	 * @param gaps
	 */
	public void setGaps(int gaps) {
		this.gaps= gaps;
	}

	/**
	 * Sets the number of identical residues or bases in the two sequences
	 * @param identity
	 */
	public void setIdentity(int identity) {
		this.identity = identity;
	}

	/**
	 * Sets the number of similar residues or bases in the two sequences
	 * @param similarity
	 */
	public void setSimilarity(int similarity) {
		this.similarity = similarity;
	}
	/**
	 * Returns the offset for the 1<sup>st</sup> sequence
	 * @return the offset start of the aligned 1st sequence
	 */
	public int getOffset1() {
		return offset1;
	}

	/**
	 * Returns the offset for the 2<sup>nd</sup> sequence
	 * @return the offset start of the aligned 2nd sequence
	 */
	public int getOffset2() {
		return offset2;
	}

	/**
	 * Sets the offset for the 1<sup>st</sup> sequence
	 * @param the offset start of the aligned 1st sequence
	 */
	public void setOffset1(int offset) {
		offset1 = offset;
	}

	/**
	 * Sets the offset for the 2<sup>nd</sup> sequence
	 * @param the offset start of the aligned 2nd sequence
	 */
	public void setOffset2(int offset) {
		offset2 = offset;
	}
	
	/**
	 * Formats the alignment to the <a href="http://www.hgmp.mrc.ac.uk/Software/EMBOSS/Themes/AlignExamples/align.pair">pair</a> format
	 * @return a pair representation of the alignment
	 */
	public String toPair( ) {
		return Pair.format(this);
	}
}
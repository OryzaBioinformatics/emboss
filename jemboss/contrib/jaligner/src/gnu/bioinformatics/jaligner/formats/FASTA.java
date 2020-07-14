/**
 * @author Ahmed Moustafa (ahmed at users.sourceforge.net)
 * $Id: FASTA.java,v 1.3 2003/09/24 09:07:40 timc Exp $
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

package gnu.bioinformatics.jaligner.formats;

/**
 * <a href="http://www.ncbi.nlm.nih.gov/BLAST/fasta.html">FASTA</a> format.
 * 
 * @version $Revision: 1.3 $ 
 * @author Ahmed Moustafa
 */

public class FASTA {
	
	public static final int LINE_LENGTH = 60;		// Number of characters per line
	private	String name = "";						// Sequence name
	private String description = "";				// Sequence description
	private char[] sequence = null;					// Sequence characters

	/**
	 * Constructor for FASTA.
	 */
	public FASTA() {
		super();
	}

	/**
	 * Returns the description.
	 * @return String
	 */
	public String getDescription() {
		return description == null ? "" : description;
	}

	/**
	 * Returns the name.
	 * @return String
	 */
	public String getName() {
		return name == null ? "" : name;
	}

	/**
	 * Returns the sequence.
	 * @return String
	 */
	public char[] getSequence() {
		return sequence;
	}

	/**
	 * Sets the description.
	 * @param description The description to set
	 */
	public void setDescription(String description) {
		this.description= description;
	}

	/**
	 * Sets the name.
	 * @param name The name to set
	 */
	public void setName(String name) {
		this.name= name;
	}

	/**
	 * Sets the sequence.
	 * @param sequence The sequence to set
	 */
	public void setSequence(char[] sequence) {
		this.sequence= sequence;
	}

	/**
	 * Returns the name, description and sequence combined in one string.
	 * The length of each line in the sequence is gnu.bioinformatics.gnu.bioinformatics.jaligner.formats.FASTA.LINE_LENGTH
	 * 
	 * @return String
	 */
	public String toString( ) {
		StringBuffer buffer = new StringBuffer (">");
		buffer.append(name == null ? "" : name);
		buffer.append(description == null || description.length() == 0 ? ""  : " " + description);
		buffer.append("\n");
        for (int i = 0, n = sequence.length; i * LINE_LENGTH < n; i++) {
        	for (int j = i * LINE_LENGTH, m = (i + 1) * LINE_LENGTH < n ? (i + 1) * LINE_LENGTH: n; j < m; j++) {
        		buffer.append(sequence[j]);
        	}
        	buffer.append("\n");
        }
		return buffer.toString( );
	}
}
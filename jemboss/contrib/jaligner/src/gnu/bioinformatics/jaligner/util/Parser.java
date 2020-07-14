/**
 * @author Ahmed Moustafa (ahmed at users.sourceforge.net)
 * $Id: Parser.java,v 1.3 2003/09/24 09:07:40 timc Exp $
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

import gnu.bioinformatics.jaligner.exceptions.ParserException;
import gnu.bioinformatics.jaligner.formats.FASTA;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;

/**
 * A parser to sequences from different formats.
 * <br>
 * Currently the supported formats are:
 * <ul>
 * 	<li><a href="http://www.ncbi.nlm.nih.gov/BLAST/fasta.html">FASTA</a></li>
 * </ul>
 *
 * @author Ahmed Moustafa (ahmed at users.sourceforge.net) 
 * @version $Revision: 1.3 $
 */

public class Parser {
	
	/**
	 * Returns a parsed FASTA from a string.
	 * 
	 * @param sequence string to be paresed
	 * @return FASTA
	 * @throws ParserException
	 */
	public static FASTA parseFASTA (String sequence) throws ParserException {
		
		if (sequence == null)
			throw new ParserException ( "Null FASTA" );
			
		sequence = sequence.trim();
	
		if (sequence.length() == 0)
			throw new ParserException ( "Empty FASTA" );
				
		if (!sequence.startsWith(">"))
			throw new ParserException ( "Does not start with \">\"" );
					
		int index = sequence.indexOf("\n");
			
		if (index == -1) {
			throw new ParserException ( "No sequence" );
		}
			
		String first = sequence.substring(1, index);
		sequence = sequence.substring(index).toUpperCase();
			
		index = 0;
		for (int i = 0; i < first.length() && first.charAt(i) != ' ' && first.charAt(i) != '\t'; i++, index++);
		String sequenceName = first.substring(0, index);
		String sequenceDescription = index + 1 > first.length() ? "" : first.substring(index + 1);
		
		char[] buffer = prepareSequence(sequence.toUpperCase());

    	FASTA fasta = new FASTA();
    	fasta.setDescription(sequenceDescription);
    	fasta.setName(sequenceName);
    	fasta.setSequence(buffer);
	    	
    	return fasta;
	}

	/**
	 * Returns a FASTA parsed and loaded an input stream
	 * @param is FASTA format input stream 
	 * @return FASTA
	 * @throws IOException
	 * @throws ParserException
	 */
	public static FASTA loadFASTA (InputStream is) throws IOException, ParserException {
		BufferedReader reader = new BufferedReader(new InputStreamReader(is));
		
		// Read & parse the first line
		String line = reader.readLine();
			
		if (!line.startsWith(">"))
			throw new ParserException ( "No \">\"" );
			
		line = line.substring(1).trim();
		int index = 0;
		for (int i = 0; i < line.length() && line.charAt(i) != ' ' && line.charAt(i) != '\t'; i++, index++);
		
		String sequenceName = line.substring(0, index);
		String sequenceDescription = index + 1 > line.length() ? "" : line.substring(index + 1);
		
		// Read the remaining the file (the actual sequence)
        
		StringBuffer buffer = new StringBuffer();
		while ((line = reader.readLine()) != null) {
			buffer.append(line);
		}
		reader.close();
        
		FASTA fasta = new FASTA();
		fasta.setDescription(sequenceDescription);
		fasta.setName(sequenceName);
		fasta.setSequence(prepareSequence(buffer.toString().toUpperCase()));
	    	
		return fasta;
	}

	/**
	 * Returns a FASTA parsed and loaded from a file
	 * @param file FASTA format file
	 * @return FASTA
	 * @throws FileNotFoundException
	 * @throws IOException
	 * @throws ParserException
	 */
	public static FASTA loadFASTA (File file) throws FileNotFoundException, IOException, ParserException {
		return loadFASTA(new FileInputStream(file));
	}

	/**
	 * Returns a FASTA parsed and loaded from a file
	 * @param filename file name of FASTA format file
	 * @return FASTA
	 * @throws FileNotFoundException
	 * @throws IOException
	 * @throws ParserException
	 */
	public static FASTA loadFASTA (String filename) throws Exception {
		return loadFASTA(new FileInputStream(filename));
	}
	
	/**
	 * Removes whitespaces from a sequence and validates the remaining characters.
	 * 
	 * @param sequence sequence to be prepared
	 * @return prepared array of characters
	 * @throws ParserException
	 */
	public static char[] prepareSequence (String sequence) throws ParserException {
		return prepareSequence(sequence.toCharArray());
	}
	
	/**
	 * Removes whitespaces from a sequence and validates the remaining characters.
	 * 
	 * @param sequence sequence to be prepared
	 * @return prepared array of characters
	 * @throws ParserException
	 */
	public static char[] prepareSequence (char[] sequence) throws ParserException {
		char[] buffer1 = new char[sequence.length];
		
		int length = 0;
		for (int i = 0, n = sequence.length; i < n; i++) {
			switch ( sequence[i] ) {
				// skip whitespaces
				case 9:
				case 10:
				case 13:
				case 32: break;
				
				// add a valid character
				case 'A':
				case 'B':
				case 'C':
				case 'D':
				case 'E':
				case 'F':
				case 'G':
				case 'H':
				case 'I':
				case 'K':
				case 'L':
				case 'M':
				case 'N':
				case 'P':
				case 'Q':
				case 'R':
				case 'S':
				case 'T':
				case 'U':
				case 'V':
				case 'W':
				case 'Y':
				case 'Z':
				case 'X':
				
				case '-':
				case '*': buffer1[length++] = sequence[i]; break;
							
				// throw an exception for anything else
				default: throw new ParserException ( "Invalid sequence character: " + sequence[i] ); 
			}
		}
		
		char[] buffer2 = new char[length];
		for (int i = 0; i < length; i++) {
			buffer2[i] = buffer1[i];
		}
		
		return buffer2;
	}
}
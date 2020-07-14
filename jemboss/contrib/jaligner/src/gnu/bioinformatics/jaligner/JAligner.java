/**
 * $Id: JAligner.java,v 1.1 2003/09/24 09:09:32 timc Exp $
 * @author Ahmed Moustafa (ahmed at users.sourceforge.net)
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

package gnu.bioinformatics.jaligner;

import gnu.bioinformatics.jaligner.algorithms.SmithWaterman;
import gnu.bioinformatics.jaligner.exceptions.ParserException;
import gnu.bioinformatics.jaligner.formats.FASTA;
import gnu.bioinformatics.jaligner.formats.Pair;
import gnu.bioinformatics.jaligner.gui.JAlignerWindow;
import gnu.bioinformatics.jaligner.util.Alignment;
import gnu.bioinformatics.jaligner.util.Matrices;
import gnu.bioinformatics.jaligner.util.Parser;

import java.io.File;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.io.InputStream;
import java.text.DecimalFormat;
import java.util.Date;

/**
 * Provides command line interface if arguments are passed, otherwise the GUI will be instanitated.
 * 
 * @author Ahmed Moustafa (ahmed at users.sourceforge.net)
 * @version $Revision: 1.1 $
 */

public class JAligner {
	
	public static final String ALGORITHM_SMITH_WATERMAN = "sw"; 

	public static void main(String[] args) {
		try {
			if (args.length == 0) {
				new JAlignerWindow().show(); 
			} else if (args[0].equalsIgnoreCase(ALGORITHM_SMITH_WATERMAN)) {
				if (args.length != 6) {
					System.err.println( "Invalid number of arguments for Smith-Waterman algorithm" );
					printUsage();
					System.exit(1);
				}
				String s1 = args[1];
				String s2 = args[2];
				String matrix = args[3];
				float open = Float.parseFloat(args[4]);
				float extend = Float.parseFloat(args[5]);
				
				FASTA fasta1 = Parser.loadFASTA(s1);
				FASTA fasta2 = Parser.loadFASTA(s2);
				
				printHeader ();
				
				Alignment alignment = sw (fasta1, fasta2, matrix, open, extend);
				printStatistics (alignment, matrix, open, extend);
				Pair.print(alignment);
				
				printTailer();
			} else {
				System.err.println( "Invalid algorithm: " + args[0]);
				printUsage();
				System.exit(1);
			}
		} catch (Exception e) {
			e.printStackTrace();
			System.exit(1);
		}
	}

	/**
	 * Prints the usage of the SW through the command line 
	 */
	private static void printUsage( ) {
		System.out.println ( ) ;
		System.out.println ( JAligner.class.getName() );
		System.out.println ( "$Revision: 1.1 $" );
		System.out.println ( ) ;
		System.out.println ( "Usage:" );
		System.out.println ( "------" );
		System.out.println ( "Smith-Waterman:" ) ;
		System.out.println ( "---------------" ) ;
		System.out.println ( "java -classpath jaligner.jar " + JAligner.class.getName() + " sw <s1> <s2> <matrix> <open> <extend>" );
		System.out.println ( ) ;
		System.out.println ( "Example:" );
		System.out.println ( "--------" );
		System.out.println ( "java -classpath jaligner.jar " + JAligner.class.getName() + " sw s1.fasta s2.fasta BLOSUM62 10.0 0.5" );
		System.out.println ( ) ;
	}

	/**
	 * Prints header information
	 * @param output output file name
	 */
	private static void printHeader ( ) {
		System.out.println("########################################");
		System.out.println("# Program:\t" + JAligner.class.getName() );
		System.out.println("# Rundate:\t" + new Date().toString ( ));
		System.out.println("# Report_file:\tstdout" );
		System.out.println("########################################");
	}

	/**
	 * Prints statistics
	 * @param alignment alignment result
	 * @param matrix scoring matrix
	 * @param open open gap penalty
	 * @param extend extend gap penalty
	 */
	private static void printStatistics (Alignment alignment, String matrix, float open, float extend) {
		DecimalFormat f1 = new DecimalFormat( "0.00" );
		DecimalFormat f2 = new DecimalFormat( "0.00%" );

		int length = alignment.getSequence1().length;
		int identity = alignment.getIdentity();
		int similarity = alignment.getSimilarity();
		int gaps = alignment.getGaps();
		float score = alignment.getScore();

		System.out.println("#=======================================");
		System.out.println("#");
		System.out.println("# Aligned_sequences: 2");
		System.out.println("# 1: " + alignment.getName1());
		System.out.println("# 2: " + alignment.getName2());
		System.out.println("# Matrix: " + matrix);
		System.out.println("# Gap_penalty: " + open);
		System.out.println("# Extend_penalty: " + extend);
		System.out.println("#");
		System.out.println("# Length: " + length );
		System.out.println("# Identity: " +  identity + "/" + length + " (" + f2.format(identity/(float)length) + ")");  
		System.out.println("# Similarity: " + similarity + "/" + length + " (" + f2.format(similarity/(float)length) + ")");
		System.out.println("# Gaps: " + gaps + "/" + length + " (" + f2.format(gaps/(float)length) + ")");
		System.out.println("# Score: " + f1.format(alignment.getScore()));
		System.out.println("#");
		System.out.println("#");
		System.out.println("#=======================================");
	}

	/**
	 * Prints tailer information
	 */
	private static void printTailer ( ) {
		System.out.println("#---------------------------------------");
		System.out.println("# Finished:\t" + new Date().toString ( ) );
		System.out.println("#---------------------------------------");
	}

	/**
	 * Aligns two sequences by Smith-Waterman algorithm
	 * @param is1 input stream for the 1<sup>st</sup> sequence in FASTA format
	 * @param is2 input stream for the 2<sup>nd</sup> sequence in FASTA format
	 * @param matrix scoring matrix
	 * @param open open gap penalty
	 * @param extend extend gap penalty
	 * @return alignment object contains the two aligned sequences, the alignment score and alignment statistics
	 * @throws Exception
	 */
	public static Alignment sw (InputStream is1, InputStream is2, String matrix, float open, float extend) throws ParserException, FileNotFoundException, IOException {
		FASTA fasta1 = Parser.loadFASTA(is1);
		FASTA fasta2 = Parser.loadFASTA(is2);
		return sw (fasta1, fasta2, matrix, open, extend);
	}
	
	/**
	 * Aligns two sequences by Smith-Waterman algorithm
	 * @param file1 1<sup>st</sup> sequence in FASTA format
	 * @param file2 2<sup>nd</sup> sequence in FASTA format
	 * @param matrix scoring matrix
	 * @param open open gap penalty
	 * @param extend extend gap penalty
	 * @return alignment object contains the two aligned sequences, the alignment score and alignment statistics
	 * @throws Exception
	 */
	public static Alignment sw (File file1, File file2, String matrix, float open, float extend) throws ParserException, FileNotFoundException, IOException {
		FASTA fasta1 = Parser.loadFASTA(file1);
		FASTA fasta2 = Parser.loadFASTA(file2);
		return sw (fasta1, fasta2, matrix, open, extend);
	}

	/**
	 * Aligns two sequences by Smith-Waterman algorithm
	 * @param fasta1 1<sup>st</sup> sequence in FASTA format
	 * @param fasta2 2<sup>nd</sup> sequence in FASTA format
	 * @param matrix scoring matrix
	 * @param open open gap penalty
	 * @param extend extend gap penalty
	 * @return alignment object contains the two aligned sequences, the alignment score and alignment statistics
	 * @throws IOException
	 * @throws FileNotFoundException
	 */
	public static Alignment sw (FASTA fasta1, FASTA fasta2, String matrix, float open, float extend) throws ParserException, FileNotFoundException, IOException {
		char[] sequence1 = fasta1.getSequence ( );
		char[] sequence2 = fasta2.getSequence ( );
		Alignment alignment = sw(sequence1, sequence2, matrix, open, extend);
		alignment.setName1(fasta1.getName ( ));
		alignment.setName2(fasta2.getName ( ));
		return alignment;
	}
	
	/**
	 * Aligns two sequences by Smith-Waterman algorithm
	 * @param sequence1 1<sup>st</sup> sequence in plain format
	 * @param sequence2 2<sup>nd</sup> sequence in plain format
	 * @param matrix scoring matrix
	 * @param open open gap penalty
	 * @param extend extend gap penalty
	 * @return alignment object contains the two aligned sequences, the alignment score and alignment statistics
	 * @throws ParserException
	 * @throws IOException
	 * @throws FileNotFoundException
	 */
	public static Alignment sw (String sequence1, String sequence2, String matrix, float open, float extend) throws ParserException, FileNotFoundException, IOException {
		char[] array1 = Parser.prepareSequence(sequence1); 
		char[] array2 = Parser.prepareSequence(sequence2);
		return SmithWaterman.align(array1, array2, Matrices.getInstance().get(matrix), open, extend);
	}

	/**
	 * Aligns two sequences by Smith-Waterman algorithm
	 * @param sequence1 1<sup>st</sup> sequence in plain format
	 * @param sequence2 2<sup>nd</sup> sequence in plain format
	 * @param matrix scoring matrix
	 * @param open open gap penalty
	 * @param extend extend gap penalty
	 * @return alignment object contains the two aligned sequences, the alignment score and alignment statistics
	 * @throws ParserException
	 * @throws IOException
	 * @throws FileNotFoundException
	 */
	public static Alignment sw (char[] sequence1, char[] sequence2, String matrix, float open, float extend) throws ParserException, FileNotFoundException, IOException {
		char[] array1 = Parser.prepareSequence(sequence1); 
		char[] array2 = Parser.prepareSequence(sequence2);
		return SmithWaterman.align(array1, array2, Matrices.getInstance().get(matrix), open, extend);
	}
}

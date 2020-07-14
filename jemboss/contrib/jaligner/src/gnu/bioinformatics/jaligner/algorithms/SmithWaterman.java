/**
 * $Id: SmithWaterman.java,v 1.3 2003/09/24 09:07:40 timc Exp $
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

package gnu.bioinformatics.jaligner.algorithms;

import gnu.bioinformatics.jaligner.util.Alignment;

/**
 * An implementation of the Smith-Waterman algorithm for pairwise sequence local alignment.
 * The basic algorithm is modified to support affine gap penatlies with Gotoh's improvement.
 * 
 * @author Ahmed Moustafa (ahmed at users.sourceforge.net)
 * @version $Revision: 1.3 $
 */

public class SmithWaterman {
	/**
	 * Diagonal direction
	 */
	private static final byte DIRECTION_DIAGONAL = 0;
	
	/**
	 * Left direction
	 */
	private static final byte DIRECTION_LEFT = 1;
	
	/**
	 * Up direction
	 */
	private static final byte DIRECTION_UP = 2;
	
	/**
	 * Maximum score of the alignment
	 */
	private float score = Float.NEGATIVE_INFINITY;
	
	/**
	 * Row of the cell with the maximum alignment score
	 */
	private int startTracebackRow;
	
	/**
	 * Column of the cell with the maximum alignment score
	 */
	private int startTracebackCol;
	
	/**
	 * Constructor for SmithWaterman
	 */
	private SmithWaterman( ) {
		super( );
	}

	/**
	 * Aligns two sequences by Smith-Waterman algorithm
	 * @param sequence1 1<sup>st</sup> sequence
	 * @param sequence2 2<sup>nd</sup> sequence
	 * @param matrix scoring matrix
	 * @param open open gap penalty
	 * @param extend extend gap penalty
	 * @return alignment object contains the two aligned sequences, the alignment score and alignment statistics
	 */
	public static Alignment align (final char[] sequence1, final char[] sequence2, final float[][] matrix, final float open, final float extend) {
		System.out.println ( "Aligning with Smith-Waterman (" + sequence1.length + " by " + sequence2.length + ") ..." );
		System.out.println ( "$Revision: 1.3 $" );
		long start = System.currentTimeMillis ( );
		SmithWaterman instance = new SmithWaterman( );
		byte[] pointers = instance.construct(sequence1, sequence2, matrix, open, extend);
		Alignment alignment = instance.traceback(sequence1, sequence2, matrix, pointers);
		long end = System.currentTimeMillis ( );
		System.out.println ( "Finished aligning with Smith-Waterman in " + (end-start) + " milliseconds");
		return alignment;
	}

	/**
	 * Constructs a pointers matrix for traceback
	 * @param sequence1 1<sup>st</sup> sequence
	 * @param sequence2 2<sup>nd</sup> sequence
	 * @param matrix scoring matrix
	 * @param open open gap penalty
	 * @param extend extend gap penalty
	 * @return matrix of pointers for traceback
	 */
	private byte[] construct(final char[] sequence1, final char[] sequence2, final float[][] matrix, final float open, final float extend) {
		int m = sequence1.length + 1;
		int n = sequence2.length + 1;
		
		float[] similarity = new float[n];
		float[] vertical = new float[n];
		float[] horizontal = new float[n];
		
		int currentTracebackRow = 0;
		int currentTracebackCol = 0;
		float currentScore = Float.NEGATIVE_INFINITY;
		
		similarity[0] = vertical[0] = horizontal[0] = 0;
		
		byte[] pointers = new byte[m * n];
		
		float a, b, c, s, o, e, diagonal = 0;
		
		for (int i = 1, row = n; i < m; i++, row += n) {
			for (int j = 1; j < n; j++) {
				
				a = diagonal + matrix[sequence1[i - 1]][sequence2[j - 1]];
				
				o = similarity[j] - open;
				e = vertical[j] - extend;
				b = vertical[j] = o > e ? o : e;
				
				o = similarity[j - 1] - open;
				e = horizontal[j - 1] - extend;
				c = horizontal[j - 1] = o > e ? o : e;

				if (a > b) {
					if (a > c) {
						s = a;
						pointers[row + j] = DIRECTION_DIAGONAL;
					} else {
						s = c;
						pointers[row + j] = DIRECTION_LEFT;
					}
				} else if (c > b) {
					s = c;
					pointers[row + j] = DIRECTION_LEFT;
				} else {
					s = b;
					pointers[row + j] = DIRECTION_UP;
				}

				diagonal = similarity[j];
				similarity[j] = s > 0 ? s : 0;

				if (s > currentScore) {
					currentScore = s;
					currentTracebackRow = i;
					currentTracebackCol = j;
				}
			}
		}
		
		score = currentScore;
		startTracebackRow = currentTracebackRow;
		startTracebackCol = currentTracebackCol;
		
		return pointers;
	}

	/**
	 * Returns alignment for two sequences based on the passed array of pointers
	 * @param sequence1 1<sup>st</sup> sequence
	 * @param sequence2 2<sup>nd</sup> sequence
	 * @param pointers matrix of pointers
	 * @return alignment object contains the two aligned sequences, the alignment score and alignment statistics 
	 */
	private Alignment traceback(final char[] sequence1, final char[] sequence2, final float[][] matrix, final byte[] pointers) {

		int maxlen = sequence1.length + sequence2.length;
		char[] reversedSequenceA = new char[maxlen];
		char[] reversedSequenceB = new char[maxlen];
		char[] reversedMarkup = new char[maxlen];
		
		int len1 = 0;
		int len2 = 0;
		int len3 = 0;
		
		int identity = 0;
		int similarity = 0;
		int gaps = 0;
		
		char c1, c2;

		int currentRow = startTracebackRow;
		int currentCol = startTracebackCol;
		int n = sequence2.length + 1;
		int row = currentRow * n;
		

		// Start the traceback
		while (currentRow != 0 && currentCol != 0) {
			switch (pointers[row + currentCol]) {
				case DIRECTION_UP			:	reversedSequenceA[len1++] = sequence1[currentRow-1];
									reversedSequenceB[len2++] = Alignment.GAP;
									reversedMarkup[len3++] = Alignment.MARKUP_GAP;
									gaps++;
									currentRow--;
									row -= n;
									break;

				case DIRECTION_DIAGONAL	:	c1 = sequence1[currentRow-1];
									c2 = sequence2[currentCol-1];
									reversedSequenceA[len1++] = c1;
									reversedSequenceB[len2++] = c2;
									if (c1 == c2) {
										reversedMarkup[len3++] = Alignment.MARKUP_IDENTITY;
										identity++;
										similarity++;
									} else if (matrix[c1][c2] > 0) {
										reversedMarkup[len3++] = Alignment.MARKUP_SIMILARITY;
										similarity++;
									} else {
										reversedMarkup[len3++] = Alignment.MARKUP_MISMATCH;
									}
									currentRow--;
									currentCol--;
									row -= n;
									break;

				case DIRECTION_LEFT		:	reversedSequenceA[len1++] = Alignment.GAP;
									reversedSequenceB[len2++] = sequence2[currentCol-1];
									reversedMarkup[len3++] = Alignment.MARKUP_GAP;
									gaps++;
									currentCol--;
									break;
			}
		}

		Alignment alignment = new Alignment();
		alignment.setScore(score);
		alignment.setIdentity(identity);
		alignment.setSimilarity(similarity);
		alignment.setGaps(gaps);
		alignment.setOffset1(currentRow);
		alignment.setOffset2(currentCol);
		
		char[] aligned1 = new char[len1];
		char[] aligned2 = new char[len2];
		char[] markup = new char[len3];
		
		for (int i = len1 - 1, j = 0; i >= 0; i--, j++) {
			aligned1[j] = reversedSequenceA[i];
		}
		
		for (int i = len2 - 1, j = 0; i >= 0; i--, j++) {
			aligned2[j] = reversedSequenceB[i];
		}
		
		for (int i = len3 - 1, j = 0; i >= 0; i--, j++) {
			markup[j] = reversedMarkup[i];
		}
		
		alignment.setSequence1(aligned1);
		alignment.setSequence2(aligned2);
		alignment.setMarkup(markup);

		return alignment;
	}
}
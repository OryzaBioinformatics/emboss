/**
 * @author Ahmed Moustafa (ahmed at users.sourceforge.net)
 * $Id: Matrices.java,v 1.3 2003/09/24 09:07:40 timc Exp $
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

import java.io.BufferedReader;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.net.JarURLConnection;
import java.net.URL;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Enumeration;
import java.util.HashMap;
import java.util.StringTokenizer;
import java.util.jar.JarEntry;
import java.util.jar.JarFile;

/**
 * A holder for the scoring matrices (Singleton).
 * 
 * @author Ahmed Moustafa (ahmed at users.sourceforge.net)
 * @version $Revision: 1.3 $
 */

public class Matrices extends HashMap {
	
	/**
	 * An instance of the Matrices, there is a single instance per JVM.
	 */
	private static Matrices instance;
	
	/**
	 * The starter character of a comment line.
	 */
	private static final char COMMENT_STARTER = '#';
	
	/**
	 * The size of the scoring matrix. It is the number of the characters in the ASCII table.
	 * It is more than the 20 amino acids just to save the processing time of the mapping. 
	 */
	private static final int SIZE = 127;
	
	/**
	 * The path to the matrices within the package.
	 */
	private static final String MATRICES_HOME = "gnu/bioinformatics/jaligner/matrices/";

	/**
	 * Constructor
	 */
	private Matrices ( ) {
	}
	
	/**
	 * Returns instance of Matrices.
	 * @return HashMap of similarity matrices
	 */
	public static Matrices getInstance ( ) {
		if (instance == null) {
			instance = new Matrices( );
		}
		return instance;
	}

	/**
	 * Loads similarity matrix from a Jar or from a file system.
	 * Puts loaded matrix into the HashMap.
	 * @param matrix name of scoring matrix
	 * @throws FileNotFoundException
	 * @throws IOException
	 */
	private void load (String matrix) throws FileNotFoundException, IOException {
		InputStream is = null;
		
		if (new StringTokenizer(matrix, System.getProperty("file.separator")).countTokens() == 1) {
			// matrix does not include the path
			// Load the matrix from matrices.jar
			is = this.getClass().getClassLoader().getResourceAsStream(MATRICES_HOME + matrix);
		} else {
			// matrix includes the path information
			// Load the matrix from the file system
			is = new FileInputStream(matrix); 
		}
		
		char[] acids = new char[SIZE];
		
		// Initialize the acids array to null values (ascii = 0)
		for (int i = 0; i < SIZE; i++) acids[i] = 0;
		
		float[][] scores = new float[SIZE][SIZE];
		
		BufferedReader reader = new BufferedReader(new InputStreamReader(is));

		String line;
		// Skip the comment lines
		while ((line = reader.readLine()) != null && line.trim().charAt(0) == COMMENT_STARTER) {
		}
				
		// Read the headers line (the letters of the acids)
		StringTokenizer tokenizer;
		tokenizer = new StringTokenizer ( line.trim( ) );
		for (int j = 0; tokenizer.hasMoreTokens(); j++) {
			acids[j] = tokenizer.nextToken().charAt(0);
		}
				
		// Read the scores
		while ((line = reader.readLine()) != null) {
			tokenizer = new StringTokenizer ( line.trim( ) );
			char acid = tokenizer.nextToken().charAt(0);
			for (int i = 0; i < SIZE; i++) {
				if (acids[i] != 0) {
					scores[acid][acids[i]] = Float.parseFloat(tokenizer.nextToken()); 
				}
			}
		}
			
		put(matrix, scores);
	}
	
	/**
	 * Returns similarity matrix.
	 * @param matrix name of scoring matrix
	 * @return similarity matrix
	 * @throws FileNotFoundException
	 * @throws IOException
	 */
	public float[][] get(String matrix) throws FileNotFoundException, IOException {
		if (!containsKey(matrix)) {
			load(matrix);
		}
		return (float[][]) get((Object)matrix);
	}
	
	/**
	 * Returns similarity score between two acids from a scoring matrix.
	 * @param matrix name of scoring matrix
	 * @param c1 1<sup>st</sup> acid
	 * @param c2 2<sup>nd</sup> acid
	 * @return similarity score
	 * @throws FileNotFoundException
	 * @throws IOException
	 */
	public static float score (String matrix, char c1, char c2)  throws FileNotFoundException, IOException {
		Matrices matrices = Matrices.getInstance();
		float[][] similarity = matrices.get(matrix);
		return similarity[c1][c2];
	}
	
	/**
	 * Returns a list of the scoring matrix in the matrices home
	 * @return sorted array of scoring matrices
	 * @throws IOException
	 */
	public static String[] list ( ) throws IOException {
		URL url = Matrices.class.getClassLoader().getResource(MATRICES_HOME);
		if (url.getFile().toString().indexOf("!") != -1) {
			// It is a Jar file
			JarURLConnection jarConn = (JarURLConnection)url.openConnection();
			JarFile jarfile = jarConn.getJarFile();
			Enumeration list = jarfile.entries();
			JarEntry entry;
			ArrayList array = new ArrayList();
			String name;
			int n = MATRICES_HOME.length();
			while (list.hasMoreElements()) {
				entry = (JarEntry) list.nextElement();
				if (!entry.isDirectory()) {
					name = entry.getName();
					if (name.startsWith(MATRICES_HOME)) {
						array.add(name.substring(n));
					}
				}
			}
			String[] entries = new String[array.size()];
			for (int i = 0; i < entries.length; i++) {
				entries[i] = (String) array.get(i);
			}
			Arrays.sort(entries, new AlphanumericCompartor());
			return entries;
		} else {
			// It is a file system
			File file = new File (url.getFile());
			String files[] = file.list( );
			Arrays.sort(files, new AlphanumericCompartor());
			return files;
		}
	}
}
/* @source align application
**
** Simple Smith-Waterman alignment.
** @author: Copyright (C) Ian Longden (il@sanger.ac.uk)
** @@
**
** This program is free software; you can redistribute it and/or
** modify it under the terms of the GNU General Public License
** as published by the Free Software Foundation; either version 2
** of the License, or (at your option) any later version.
** 
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
** 
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software
** Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
******************************************************************************/
#include "emboss.h"

int main (int argc, char * argv[]) {

  int i,j,top,penalty=-8,nogap;
  AjPSeq seq,seq2;
  char *s1,*s2,res[2]="-";
  int **sub;
  AjPStr str1,str2;
  AjPStr newseq1=0,newseq2=0;
  AjBool showinternals;
  int **mat=0;
  int imax=0,jmax=0,max;
  AjPMatrix matrix=0;
  AjPSeqCvt cvt=0;
  static AjPStr outstr = NULL;

  ajUser("HELLO!");
  embInit("simplesw", argc, argv);

  seq = ajAcdGetSeq ("sequencea");
  seq2 = ajAcdGetSeq ("sequenceb");
  matrix  = ajAcdGetMatrix("datafile");
  showinternals = ajAcdGetBool("showinternals");

  sub = ajMatrixArray(matrix);
  cvt = ajMatrixCvt(matrix);

  str1 = ajSeqStr(seq);
  ajStrToUpper(&str1);
  str2 = ajSeqStr(seq2);
  ajStrToUpper(&str2);

  ajUser("HELLO! %d %d",ajSeqLen(seq),ajSeqLen(seq2));

  /* get an array of the correct size */
  AJCNEW(mat, ajSeqLen(seq2)+1);
  for(i=0;i<ajSeqLen(seq2)+1;i++){
    AJCNEW(mat[i], ajSeqLen(seq)+1);
  }

  /* set initial values to 0, rest will be set later */
  for(i=0;i<ajSeqLen(seq2)+1;i++)
    mat[i][0] = 0;
  for(i=0;i<ajSeqLen(seq)+1;i++)
    mat[0][i] = 0;

  s1 = ajStrStr(str2);

  for(i=1;i<ajSeqLen(seq2)+1;i++){
    s2 = ajStrStr(str1);
    for(j=1;j<ajSeqLen(seq)+1;j++){
      if(mat[i-1][j] > mat[i][j-1])
	top = mat[i-1][j];
      else
	top = mat[i][j-1];
      top += penalty;
      
      nogap = mat[i-1][j-1]+sub[ajSeqCvtK(cvt,*s1)][ajSeqCvtK(cvt,*s2)];
      ajDebug("i: %d j: %d '%c' '%c' sub[%d][%d] = %d\n",
	      i, j, *s1, *s2, ajSeqCvtK(cvt,*s1), ajSeqCvtK(cvt,*s2),
	      sub[ajSeqCvtK(cvt,*s1)][ajSeqCvtK(cvt,*s2)]);
      s2++;
      
      if(top < nogap)
	top = nogap;
      if(top>0)
	mat[i][j]=top;
      else
	mat[i][j] = 0;
    }
    s1++;
  }

  if(showinternals){
    for(i=0;i<ajSeqLen(seq2)+1&&i<40;i++){
      for(j=0;j<ajSeqLen(seq)+1&&j<40;j++)
	ajFmtPrintAppS(&outstr, " %4d",mat[i][j]);
      ajUser("%S", outstr);
      ajStrDelReuse (&outstr);
    }
  }    

  max = 0;
  for(i=0;i<ajSeqLen(seq2)+1;i++){
    for(j=0;j<ajSeqLen(seq)+1;j++){
      if(mat[i][j] > max){
	max = mat[i][j];
	imax = i;
	jmax = j;
      }
    }
  }

  if(showinternals)
    ajUser("max-> mat[%d][%d] = %d",imax,jmax,mat[imax][jmax]);

  /* now build up the new seqs with gaps */

  i = imax;
  j = jmax;
  s1 = ajStrStr(ajSeqStr(seq));
  s1 += jmax-1;
  s2 = ajStrStr(ajSeqStr(seq2));
  s2 += imax-1;

  while(mat[i][j]){
    if((mat[i][j] - sub[ajSeqCvtK(cvt,*s1)][ajSeqCvtK(cvt,*s2)]) ==
       mat[i-1][j-1]){
      /* match */
      res[0] = *s1;
      ajStrInsertC(&newseq1,0,res);
      res[0] = *s2;
      ajStrInsertC(&newseq2,0,res);
      s1--;
      s2--;
      i--;j--;
    }
    else if(mat[i][j] - penalty == mat[i][j-1]){
      res[0] = *s1;
      ajStrInsertC(&newseq1,0,res);
      res[0]='-';
      ajStrInsertC(&newseq2,0,res);
      s1--;
      j--;
    }
    else {
      res[0] = *s2;
      ajStrInsertC(&newseq2,0,res);
      res[0]='-';
      ajStrInsertC(&newseq1,0,res);
      s2--;
      i--;
    }
  }
  ajUser("*%S*",newseq1);
  ajUser("*%S*",newseq2);
  ajExit();
  return 0;
}









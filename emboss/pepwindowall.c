/*  Last edited: Jan 24 15:05 2000 (pmr) */
/* @source pepwindowall application
**
** Displays protein hydropathy.
** @author: Copyright (C) Ian Longden (il@sanger.ac.uk)
** @@
** Original program by Jack Kyte and Russell F. Doolittle.
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
/* -datafile="/nfs/adnah/il/emboss/test/data/nakai.dat" /nfs/adnah/il/emboss/test/data/globin.msf*/

#include "limits.h"
#include <float.h>
#include "emboss.h"

#define AZ 28

AjPStr getnakaidata(AjPFile file, float matrix[]){
  AjPStr buffer = NULL;
  AjPStr buf2 = NULL;
  AjPStr delim = NULL; 
  AjPStr description = NULL;
  AjPStrTok token;
  int line =0;
  char *ptr;
  int cols;

  delim = ajStrNewC(" :\t\n");

  if(!file)
    return 0;
  while (ajFileGets(file,&buffer)){
    ptr = ajStrStr(buffer);
    if(*ptr == 'D') /* description */
      description = ajStrDup(buffer);
    else if(*ptr == 'I')
      line = 1;
    else if(line == 1){
      line++;
      token = ajStrTokenInit(buffer,ajStrStr(delim));
      cols = ajStrTokenCount(&buffer,ajStrStr(delim));
      ajDebug("num of cols = %d\n",cols);

      ajStrToken(&buf2,&token,ajStrStr(delim));
      ajStrToFloat(buf2,&matrix[0]);

      ajStrToken(&buf2,&token,ajStrStr(delim));
      ajStrToFloat(buf2,&matrix[17]);

      ajStrToken(&buf2,&token,ajStrStr(delim));
      ajStrToFloat(buf2,&matrix[13]);

      ajStrToken(&buf2,&token,ajStrStr(delim));
      ajStrToFloat(buf2,&matrix[3]);

      ajStrToken(&buf2,&token,ajStrStr(delim));
      ajStrToFloat(buf2,&matrix[2]);

      ajStrToken(&buf2,&token,ajStrStr(delim));
      ajStrToFloat(buf2,&matrix[16]);

      ajStrToken(&buf2,&token,ajStrStr(delim));
      ajStrToFloat(buf2,&matrix[4]);

      ajStrToken(&buf2,&token,ajStrStr(delim));
      ajStrToFloat(buf2,&matrix[6]);

      ajStrToken(&buf2,&token,ajStrStr(delim));
      ajStrToFloat(buf2,&matrix[7]);

      ajStrToken(&buf2,&token,ajStrStr(delim));
      ajStrToFloat(buf2,&matrix[8]);

    }
    else if(line == 2){
      line++;
      token = ajStrTokenInit(buffer,ajStrStr(delim));
      cols = ajStrTokenCount(&buffer,ajStrStr(delim));
      ajStrToken(&buf2,&token,ajStrStr(delim));
      ajStrToFloat(buf2,&matrix[11]);

      ajStrToken(&buf2,&token,ajStrStr(delim));
      ajStrToFloat(buf2,&matrix[10]);

      ajStrToken(&buf2,&token,ajStrStr(delim));
      ajStrToFloat(buf2,&matrix[12]);

      ajStrToken(&buf2,&token,ajStrStr(delim));
      ajStrToFloat(buf2,&matrix[5]);

      ajStrToken(&buf2,&token,ajStrStr(delim));
      ajStrToFloat(buf2,&matrix[15]);

      ajStrToken(&buf2,&token,ajStrStr(delim));
      ajStrToFloat(buf2,&matrix[18]);

      ajStrToken(&buf2,&token,ajStrStr(delim));
      ajStrToFloat(buf2,&matrix[19]);

      ajStrToken(&buf2,&token,ajStrStr(delim));
      ajStrToFloat(buf2,&matrix[22]);

      ajStrToken(&buf2,&token,ajStrStr(delim));
      ajStrToFloat(buf2,&matrix[24]);

      ajStrToken(&buf2,&token,ajStrStr(delim));
      ajStrToFloat(buf2,&matrix[21]);

    }
  }
  return description;
}
  
int main (int argc, char * argv[]) {
  AjPFile datafile;
  AjPStr aa0str=0;
  AjPSeqset seqset;
  AjPGraphData graphdata;
  AjPGraph mult;
  char *seq;
  char *s1;
  int *position;
  int i,j,k,w;
  int midpoint,llen,maxlen;
  float total;
  float matrix[AZ];
  float min= 555.5,max = -555.5;
  
  (void) ajGraphInit("pepwindowall", argc, argv);
  
  seqset = ajAcdGetSeqset("msf");
  mult = ajAcdGetGraphxy ("graph");
  datafile  = ajAcdGetDatafile("datafile");
  llen = ajAcdGetInt("length");
  
  if(!getnakaidata(datafile,&matrix[0]))
    exit(-1);
  
  
  maxlen = ajSeqsetLen(seqset);
  aa0str = ajStrNewL(maxlen);
  midpoint = (int)((llen+1)/2);

  AJCNEW(position, ajSeqsetLen(seqset));
  
  for(i=0;i<ajSeqsetSize(seqset);i++){
    seq = ajSeqsetSeq (seqset, i);
    ajStrClear(&aa0str);
    ajDebug("HELLO (%d)   %d %d\n",
	    i,ajSeqsetSize(seqset),ajSeqsetLen(seqset));
    
    graphdata = ajGraphxyDataNewI(ajSeqsetLen(seqset));

    
    for(k=0;k<ajSeqsetLen(seqset) ;k++)
      graphdata->x[k] = FLT_MIN; 
    
    s1 = seq;
    k=0; w=0;
    while(*s1 != '\0'){
      if(ajAZToInt(*s1) != 27 ){
	ajStrAppK(&aa0str,(char)ajAZToInt(*s1));
	position[k++]= w+midpoint;
      }
      w++;
      s1++;
    }

    s1 = ajStrStr(aa0str);    
    for(j=0;j<k-llen;j++){
      total = 0;
      for(w=0;w<llen;w++){
	total = total + matrix[(int)s1[w]];
      } 
      total=total/(float)llen;
      graphdata->x[position[j]] = (float)position[j];
      graphdata->y[position[j]] = total;
      if(total > max)
	max= total;
      if(total < min)
	min = total;
      s1++;
    }
    ajGraphxyAddGraph(mult,graphdata);
  }

  min=min*1.1;
  max=max*1.1;

  ajGraphxySetGaps(mult,AJTRUE);
  ajGraphxySetOverLap(mult,AJTRUE);

  ajGraphxySetMaxMin(mult,0.0,(float)ajSeqsetLen(seqset),min,max);

  ajGraphxyDisplay(mult,AJTRUE);
  ajExit();
  return 0;
}

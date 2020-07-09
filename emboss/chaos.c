/* @source chaos application
** @author Ian Longden
** @@
** Chaos produces a chaos plot.
** The original application is part of the ACEDB genome database
** package, written by ** Richard Durbin (MRC LMB, UK)
** rd@mrc-lmba.cam.ac.uk, and Jean Thierry-Mieg (CRBM du CNRS,
** France) mieg@crbm1.cnusc.fr
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
#include "ajax.h"

int main (int argc, char *argv[]) {
  AjPSeq sequence = NULL ;
  AjPGraph graph = NULL;
  float x=0.5,y=0.5,x2;
  char *ptr;
  int i,a=0,c=0,t=0,g=0,n=0;
  char line[40];
  AjBool data=ajFalse;
  AjPFile outf=NULL;
  
  ajGraphInit ("chaos", argc, argv);
  
  sequence = ajAcdGetSeq ("sequence");
  graph = ajAcdGetGraph ("graph");
  data  = ajAcdGetBool("data");
  outf  = ajAcdGetOutfile("outfile");
  
  if(!data)
      ajGraphOpenWin(graph,-0.1,1.4,-0.1,1.1);
  else
      ajFmtPrintF(outf,"##Graphic\n##Screen x1 %f y1 %f x2 %f y2 %f\n",
		  -0.1,-0.1,1.4,1.1);

  ajSeqToLower(sequence);
  ptr = ajSeqChar(sequence);
  for(i=0;i<ajSeqLen(sequence);i++){
    if(*ptr=='a'){
      x *= 0.5;
      y *= 0.5;
      a++;
    }
    else if(*ptr=='g'){
      x = (x+ 1.0)*0.5;
      y = (y+ 1.0)*0.5;
      g++;
    }
    else if(*ptr=='c'){
      x *= 0.5;
      y = (y+1.0 )*0.5;
      c++;
    }
    else if(*ptr=='t'){
      x = (x+1.0 )*0.5;
      y *= 0.5;
      t++;
    }
    else {
      x = 0.5;
      y = 0.5;
      n++;
    }
    x2 = x + 0.0001;
    if(!data)
	ajGraphLines(&x,&y,&x2,&y,1);
    else
	ajFmtPrintF(outf,"Line x1 %f y1 %f x2 %f y2 %f colour 0\n",
		    x,y,x2,y);
    ptr++;
  }
 
  if(!data)
  {
      ajGraphTextEnd (0.0,0.0,"A");
      ajGraphTextEnd (0.0,1.0,"C");
      ajGraphTextStart (1.0,0.0,"T");
      ajGraphTextStart (1.0,1.0,"G");


      ajGraphSetCharSize(0.5);
      sprintf(line,"A %d",a);
      ajGraphTextStart (1.1,0.75,line);
      sprintf(line,"C %d",c);
      ajGraphTextStart (1.1,0.70,line);
      sprintf(line,"T %d",t);
      ajGraphTextStart (1.1,0.65,line);
      sprintf(line,"G %d",t);
      ajGraphTextStart (1.1,0.60,line);
      sprintf(line,"N %d",n);
      ajGraphTextStart (1.1,0.55,line);


      sprintf(line,"%cA %3.2f",'%',((float)a/(float)ajSeqLen(sequence))*100.0);
      ajGraphTextStart (1.1,0.45,line);
      sprintf(line,"%cC %3.2f",'%',((float)c/(float)ajSeqLen(sequence))*100.0);
      ajGraphTextStart (1.1,0.40,line);
      sprintf(line,"%cT %3.2f",'%',((float)t/(float)ajSeqLen(sequence))*100.0);
      ajGraphTextStart (1.1,0.35,line);
      sprintf(line,"%cG %3.2f",'%',((float)g/(float)ajSeqLen(sequence))*100.0);
      ajGraphTextStart (1.1,0.30,line);
      sprintf(line,"%cN %3.2f",'%',((float)n/(float)ajSeqLen(sequence))*100.0);
      ajGraphTextStart (1.1,0.25,line);

  
      ajGraphCloseWin();
  }
  else
  {
      ajFmtPrintF(outf,"Text3 x1 %f y1 %f colour 0 size 1.0 %s\n",0.0,0.0,"A");
      ajFmtPrintF(outf,"Text3 x1 %f y1 %f colour 0 size 1.0 %s\n",0.0,1.0,"C");
      ajFmtPrintF(outf,"Text1 x1 %f y1 %f colour 0 size 1.0 %s\n",1.0,0.0,"T");
      ajFmtPrintF(outf,"Text1 x1 %f y1 %f colour 0 size 1.0 %s\n",1.0,1.0,"G");
      


      sprintf(line,"A %d",a);
      ajFmtPrintF(outf,"Text1 x1 %f y1 %f colour 0 size 0.5 %s\n",
		  1.1,0.75,line);

      sprintf(line,"C %d",c);
      ajFmtPrintF(outf,"Text1 x1 %f y1 %f colour 0 size 0.5 %s\n",
		  1.1,0.70,line);

      sprintf(line,"T %d",t);
      ajFmtPrintF(outf,"Text1 x1 %f y1 %f colour 0 size 0.5 %s\n",
		  1.1,0.65,line);

      sprintf(line,"G %d",t);
      ajFmtPrintF(outf,"Text1 x1 %f y1 %f colour 0 size 0.5 %s\n",
		  1.1,0.60,line);

      sprintf(line,"N %d",n);
      ajFmtPrintF(outf,"Text1 x1 %f y1 %f colour 0 size 0.5 %s\n",
		  1.1,0.55,line);



      sprintf(line,"%cA %3.2f",'%',((float)a/(float)ajSeqLen(sequence))*100.0);
      ajFmtPrintF(outf,"Text1 x1 %f y1 %f colour 0 size 0.5 %s\n",
		  1.1,0.45,line);

      sprintf(line,"%cC %3.2f",'%',((float)c/(float)ajSeqLen(sequence))*100.0);
      ajFmtPrintF(outf,"Text1 x1 %f y1 %f colour 0 size 0.5 %s\n",
		  1.1,0.40,line);

      sprintf(line,"%cT %3.2f",'%',((float)t/(float)ajSeqLen(sequence))*100.0);
      ajFmtPrintF(outf,"Text1 x1 %f y1 %f colour 0 size 0.5 %s\n",
		  1.1,0.35,line);

      sprintf(line,"%cG %3.2f",'%',((float)g/(float)ajSeqLen(sequence))*100.0);
      ajFmtPrintF(outf,"Text1 x1 %f y1 %f colour 0 size 0.5 %s\n",
		  1.1,0.30,line);

      sprintf(line,"%cN %3.2f",'%',((float)n/(float)ajSeqLen(sequence))*100.0);
      ajFmtPrintF(outf,"Text1 x1 %f y1 %f colour 0 size 0.5 %s\n",
		  1.1,0.25,line);
  
      ajFileClose(&outf);
  }

  ajExit();
  return 0;
}


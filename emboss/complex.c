/* @source complex application
**
** Reports complexity of DNA
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

int main (int argc, char *argv[])
{
  
    char *charseq;
    char *name;
 
    AjPSeq    seq;
    AjPSeqall seqall;
    AjPSeqout seqout;
    AjPFile   outfile; 
    AjPFile   pfile;
  
    int len;
    int Num_seq = 0;

    int lwin;
    int step;
    int sim;
    int jmin;
    int jmax;
  
    AjBool print;
    AjBool freq;
    AjBool omnia;  
 
    float MedValue = 0.0; 

    embInit ("complex",argc,argv);
 
    lwin  = ajAcdGetInt ("lwin");
    step  = ajAcdGetInt ("step");
    sim   = ajAcdGetInt ("sim");
    jmin  = ajAcdGetInt ("jmin");
    jmax  = ajAcdGetInt ("jmax");
  
    omnia = ajAcdGetBool ("omnia");
    freq  = ajAcdGetBool ("freq");
    print = ajAcdGetBool ("print");
  
    seqall = ajAcdGetSeqall ("sequence");
 
    seqout = ajAcdGetSeqoutall ("outseq"); 
    outfile = ajAcdGetOutfile("outfile");
    pfile = ajAcdGetOutfile("ujtable");

    ajDebug("Output file: %F \n",outfile);

    if(omnia){ 
	embComWriteFile(outfile,jmin,jmax,lwin,step,sim);
	ajUser("do embComWriteFile\n");
	while (ajSeqallNext(seqall, &seq)) {
	    ajSeqAllWrite (seqout,seq);
	    len = ajSeqLen(seq);
	    name = ajSeqName(seq);
	    ajUser("%s %d",name,len);
	    charseq = ajSeqChar(seq);
	    if(len >= lwin){
		Num_seq++;
		embComComplexity(charseq,name,len,jmin,jmax,lwin,
				 step,sim,freq,omnia,
				 outfile,pfile,print,Num_seq,&MedValue);
		embComWriteValueOfSeq(outfile,Num_seq,name,len,MedValue);
	    }
	}
    }  
    else{
	Num_seq = 0;
	while((ajSeqallNext(seqall, &seq) && Num_seq < 1)){
	    len = ajSeqLen(seq);
	    name = ajSeqName(seq);
	    charseq = ajSeqChar(seq);
	    Num_seq ++;
	    embComComplexity(charseq,name,len,jmin,jmax,lwin,
			     step,sim,freq,omnia,
			     outfile, pfile,print,Num_seq,&MedValue);
	}
    }

        
    ajFileClose (&pfile);
  
    ajFileClose (&outfile); 

    ajUser("%d %d %d %d %d",lwin,step,sim,jmin,jmax);
    /*embComGraphComplex();*/
 
    ajExit();
    return 0;
}

/* @source cusp application
**
** Calculate codon usage table from sequence(s)
**
** @author: Copyright (C) Alan Bleasby (ableasby@hgmp.mrc.ac.uk)
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


int main(int argc, char **argv)
{
    AjPSeqall  seqall;
    AjPSeq     seq;
    AjPFile    outf;
    AjPCod     codon;
    AjPStr     substr;
    ajint beg;
    ajint end;
    ajint ccnt;
    
    
    embInit("cusp", argc, argv);

    seqall    = ajAcdGetSeqall("sequence");
    codon     = ajAcdGetCodon("cfile");
    outf      = ajAcdGetOutfile("outfile");

    ajCodClear(&codon);

    ccnt=0;
    substr=ajStrNew();
    
    while(ajSeqallNext(seqall, &seq))
    {
	beg = ajSeqallBegin(seqall);
	end = ajSeqallEnd(seqall);
	ajStrAssSub(&substr,ajSeqStr(seq),beg-1,end-1);
	ajCodCountTriplets(&codon,substr,&ccnt);
    }
    
    ajCodCalculateUsage(&codon,ccnt);

    ajFmtPrintF(outf,"# CUSP codon usage file\n");
    ajFmtPrintF(outf,"# Codon\tAmino acid\tFract   /1000\tNumber\n");
    ajCodWrite(outf,codon);
    ajFileClose(&outf);

    ajStrDel(&substr);
    ajCodDel(&codon);
    
    ajExit();
    return 0;
}

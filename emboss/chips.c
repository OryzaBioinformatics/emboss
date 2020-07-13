/* @source chips application
**
** Calculate codon usage statistics
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



/* @prog chips ****************************************************************
**
** Codon usage statistics
**
******************************************************************************/

int main(int argc, char **argv)
{
    AjPSeqall  seqall;
    AjPSeq     seq;
    AjPFile    outf;
    AjPCod     codon;
    AjPStr     substr;

    ajint ccnt;
    ajint beg;
    ajint end;

    float Nc;


    embInit("chips", argc, argv);

    seqall    = ajAcdGetSeqall("seqall");
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
	ajStrToUpper(&substr);
	ajCodCountTriplets(&codon,substr,&ccnt);
    }

    ajCodCalculateUsage(&codon,ccnt);
    Nc=ajCodCalcNc(&codon);

    ajFmtPrintF(outf,"# CHIPS codon usage statistics\n\n");
    ajFmtPrintF(outf,"Nc = %.3f\n",Nc);



    ajFileClose(&outf);

    ajCodDel(&codon);

    ajExit();
    return 0;
}

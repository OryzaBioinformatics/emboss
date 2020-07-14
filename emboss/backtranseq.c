/* @source backtranseq application
**
** Backward translation to the most likely DNA sequence
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




/* @prog backtranseq **********************************************************
**
** Back translate a protein sequence
**
******************************************************************************/

int main(int argc, char **argv)
{
    AjPSeq     a;
    AjPSeqout  outf;
    AjPCod     codon;
    AjPStr     substr;
    AjPStr     back;

    ajint beg;
    ajint end;

    embInit("backtranseq", argc, argv);

    a         = ajAcdGetSeq("sequence");
    codon     = ajAcdGetCodon("cfile");
    outf      = ajAcdGetSeqout("outfile");

    substr = ajStrNew();
    beg    = ajSeqBegin(a);
    end    = ajSeqEnd(a);
    ajStrAssSubC(&substr,ajSeqChar(a),beg-1,end-1);

    back = ajStrNew();
    ajCodSetBacktranslate(&codon);
    ajCodBacktranslate(&back,substr,codon);

    ajSeqAssSeq (a, back);

    ajSeqWrite(outf,a);

    ajStrDel(&back);
    ajStrDel(&substr);
    ajSeqWriteClose(outf);
    ajCodDel(&codon);

    ajExit();

    return 0;
}

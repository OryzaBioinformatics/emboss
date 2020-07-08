/* @source codcmp application
**
** Compare two codon usage files
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
    AjPFile    outf;
    AjPCod     first;
    AjPCod     second;
    
    double     sum;
    double     sum2;
    
    double     t;
    
    int        unused;

    int i;
    

    
    embInit("codcmp", argc, argv);

    first     = ajAcdGetCodon("first");
    second    = ajAcdGetCodon("second");
    outf      = ajAcdGetOutfile("outfile");

    
    sum    = 0.0;
    sum2   = 0.0;
    unused = 0;
    
    for(i=0;i < 64;++i)
	if(!first->fraction[i] || !second->fraction[i])
	    ++unused;
	else
	{
	    t = first->fraction[i] - second->fraction[i];
	    sum += t;
	    sum2  += t*t;
	}
	
    ajFmtPrintF(outf,"# CODCMP codon usage table comparison\n");
    ajFmtPrintF(outf,"# %s vs %s\n\n",ajStrStr(first->name),
		ajStrStr(second->name));
    
    ajFmtPrintF(outf,"Sum Difference Squared = %.3f\n",sum2);
    ajFmtPrintF(outf,"Sum Difference         = %.3f\n",sum);
    ajFmtPrintF(outf,"Codons not appearing   = %d\n",unused);

    ajFileClose(&outf);

    ajCodDel(&first);
    ajCodDel(&second);
    
    ajExit();
    return 0;
}

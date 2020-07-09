/* @source preg application
**
** Protein regular expression (perl style)
**
** @author: Copyright (C) Peter Rice
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


/* @prog preg ***************************************************************
**
** Regular expression search of a protein sequence
**
******************************************************************************/

int main(int argc, char **argv)
{

    AjPSeqall seqall;
    AjPFile outf;
    AjPRegexp patexp;
    AjPSeq seq = NULL;
    AjPStr str = NULL;
    AjPStr tmpstr = NULL;
    AjPStr substr = NULL;
    AjBool found;
    ajint ioff;
    ajint ipos;
    ajint ilen;

    embInit ("preg", argc, argv);

    outf = ajAcdGetOutfile ("outfile");
    seqall = ajAcdGetSeqall ("sequence");
    patexp = ajAcdGetRegexp ("pattern");

    ajFmtPrintF (outf, "preg search of %S with pattern %S\n", 
		 ajAcdValue("sequence"), ajAcdValue("pattern"));

    while (ajSeqallNext(seqall, &seq))
    {
	found = ajFalse;
	ipos = 1;
	ajStrAssS (&str, ajSeqStr(seq));
	ajStrToUpper(&str);
	ajDebug ("Testing '%s' len: %d %d\n",
		 ajSeqName(seq), ajSeqLen(seq), ajStrLen(str));
	while (ajRegExec (patexp, str))
	{
	    if (!found)
	    {
		ajFmtPrintF (outf, "Matches in %s\n", ajSeqName(seq));
		found = ajTrue;
	    }
	    ioff = ajRegOffset (patexp);
	    ilen = ajRegLenI (patexp, 0);
	    ajRegSubI (patexp, 0, &substr);
	    ajRegPost (patexp, &tmpstr);
	    ajStrAssS (&str, tmpstr);
	    ipos += ioff;
	    ajFmtPrintF (outf, "%15s %5d %S\n", ajSeqName(seq), ipos, substr);
	    ipos += ilen;
	}
    }

    ajFileClose (&outf);

    ajExit ();
    return 0;
}

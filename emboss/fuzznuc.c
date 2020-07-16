/* @source fuzznuc application
**
** Finds fuzzy patterns in nucleic acid sequences
** @author Copyright (C) Alan Bleasby (ableasby@hgmp.mrc.ac.uk)
** @author modified: Copyright (C) Henrikki Almusa, Medicel Oy
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
#include "stdlib.h"





/* @prog fuzznuc **************************************************************
**
** Nucleic acid pattern search
**
******************************************************************************/

int main(int argc, char **argv)
{
    AjPSeqall seqall = NULL;
    AjPSeq seq = NULL;
    AjPFeattable tab = NULL;
    AjPReport report = NULL;
    AjPStr tmpstr = NULL;

    AjBool sc;
    AjBool writeok = ajTrue;

    AjPPatlistSeq plist = NULL;

    embInit("fuzznuc", argc, argv);

    seqall   = ajAcdGetSeqall("sequence");
    report   = ajAcdGetReport("outfile");
    plist   = ajAcdGetPattern("pattern");
    sc       = ajAcdGetBoolean("complement");
 
    ajPatlistSeqDoc(plist, &tmpstr);
    ajFmtPrintAppS(&tmpstr, "\nComplement: %B\n", sc);
    ajReportSetHeader(report, tmpstr);

    writeok=ajTrue;
    while (writeok && ajSeqallNext(seqall,&seq))
    {
	tab = ajFeattableNewSeq(seq);
        embPatlistSeqSearch(tab,seq,plist,ajFalse);
        if (sc)
            embPatlistSeqSearch (tab,seq,plist,ajTrue);
        if(ajFeattableSize(tab))
	    writeok = ajReportWrite(report,tab,seq);
        ajFeattableDel(&tab);
    }

    ajPatlistSeqDel(&plist);

    ajStrDel(&tmpstr);

    ajReportSetSeqstats(report, seqall);
    ajReportClose(report);
    ajReportDel(&report);
    ajSeqallDel(&seqall);
    ajSeqDel(&seq);

    embExit();
    return 0;
}

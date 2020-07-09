#include "emboss.h"


/* @prog demoreport ***********************************************************
**
** Reads a feature table (and sequence) and writes as a report
**
******************************************************************************/

int main(int argc, char **argv)
{

    AjPReport report;
    AjPSeq seq = NULL;
    AjPFeattable ftab;

    embInit ("demoreport", argc, argv);

    report = ajAcdGetReport ("report");
    seq = ajAcdGetSeq ("sequence");

    ftab = ajSeqGetFeat(seq);

    ajReportWrite (report, ftab, seq);

    ajExit ();
    return 0;
}

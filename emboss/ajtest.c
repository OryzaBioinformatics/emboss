#include "emboss.h"

/* @prog ajtest ***************************************************************
**
** testing, and subject to frequent change
**
******************************************************************************/

int main(int argc, char **argv)
{

    AjPSeqset seqset;
    AjPSeqall seqall;
    AjPSeq seq;
    ajint i = 0;

    embInit ("ajtest", argc, argv);

    seqall = ajAcdGetSeqall ("sequence");
    seqset = ajAcdGetSeqset ("bsequence");

    ajUser ("Set of %d", ajSeqsetSize(seqset));
    while (ajSeqallNext (seqall, &seq))
    {
	ajUser ("%3d <%S>", i++, ajSeqGetUsa(seq));
    }

    ajExit ();
    return 0;
}

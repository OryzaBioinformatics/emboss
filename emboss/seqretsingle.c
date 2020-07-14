#include "emboss.h"


/* @prog seqretsingle *********************************************************
**
** Reads and writes (returns) a single sequence
**
******************************************************************************/

int main(int argc, char **argv)
{
    AjPSeqout seqout;
    AjPSeq seq = NULL;

    embInit("seqretsingle", argc, argv);

    seqout = ajAcdGetSeqout("outseq");
    seq = ajAcdGetSeq("sequence");

    ajSeqWrite(seqout, seq);
    ajSeqWriteClose(seqout);

    ajExit ();

    return 0;
}

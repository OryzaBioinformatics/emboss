#include "emboss.h"


/* @prog seqret ***************************************************************
**
** Reads and writes (returns) sequences
**
******************************************************************************/

int main(int argc, char **argv)
{

    AjPSeqout seqout;
    AjPSeq seq = NULL;

    embInit ("seqretsingle", argc, argv);

    seqout = ajAcdGetSeqoutall ("outseq");
    seq = ajAcdGetSeq ("sequence");

    ajSeqWrite (seqout, seq);
    ajSeqWriteClose (seqout);

    ajExit ();
    return 0;
}

#include "emboss.h"




static void ajtest_kim (const AjPStr seqout_name, const AjPSeq subseq);

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
    AjPStr kimout = NULL;
    AjPStr dir = NULL;

    embInit("ajtest", argc, argv);

    seqall = ajAcdGetSeqall ("sequence");
    seqset = ajAcdGetSeqset ("bsequence");
    dir = ajAcdGetDirectoryName("directory");

    ajUser("Directory '%S'", dir);
    ajUser("Set of %d", ajSeqsetSize(seqset));
    while(ajSeqallNext (seqall, &seq))
    {
	ajUser ("%3d <%S>", i++, ajSeqGetUsa(seq));
	ajFmtPrintS(&kimout, "kim%d.out", i);
	ajtest_kim (kimout, seq);
    }

    ajExit();

    return 0;
}

/* @funcstatic ajtest_kim *****************************************************
**
** Test for Kim Rutherford's reported problem
**
** @param [r] seqout_name [const AjPStr] Seqout name
** @param [r] subseq [const AjPSeq] Subsequence
** @return [void]
** @@
******************************************************************************/

static void ajtest_kim (const AjPStr seqout_name, const AjPSeq subseq)
{
    AjPFile seqout_file = ajFileNewOut(seqout_name);
    AjPSeqout named_seqout = ajSeqoutNewF(seqout_file);

    AjPStr format_str = ajStrNew();
    ajStrAssC(&format_str, "embl");

    ajSeqOutSetFormat(named_seqout, format_str);

    ajSeqWrite(named_seqout, subseq);

}

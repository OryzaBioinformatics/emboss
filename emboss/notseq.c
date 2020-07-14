/* @source notseq  application
**
** Excludes a set of sequences and writes out the remaining ones
**
** @author: Copyright (C) Gary Williams
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




static void notseq_readfile(AjPStr *pattern);




/* @prog notseq ***************************************************************
**
** Excludes a set of sequences and writes out the remaining ones
**
******************************************************************************/

int main(int argc, char **argv)
{

    AjPSeqall seqall;
    AjPSeqout seqout;
    AjPSeqout junkout;
    AjPSeq seq = NULL;
    AjPStr pattern = NULL;
    AjPStr name = NULL;
    AjPStr acc  = NULL;
    AjBool gotone=ajFalse;

    embInit("notseq", argc, argv);

    seqout  = ajAcdGetSeqoutall("outseq");
    junkout = ajAcdGetSeqoutall("junkoutseq");
    seqall  = ajAcdGetSeqall("sequence");
    pattern = ajAcdGetString("exclude");

    notseq_readfile(&pattern);

    while(ajSeqallNext(seqall, &seq))
    {
	ajStrAss(&name, ajSeqGetName(seq));
	ajStrAss(&acc, ajSeqGetAcc(seq));

	if(embMiscMatchPattern(name, pattern) ||
	    embMiscMatchPattern(acc, pattern))
	{
	    ajSeqAllWrite(junkout, seq);
	    gotone = ajTrue;
	}
	else
	    /* no match, so not excluded */
	    ajSeqAllWrite(seqout, seq);

	ajStrClear(&name);
	ajStrClear(&acc);
    }

    ajSeqWriteClose(seqout);
    ajSeqWriteClose(junkout);

    if(gotone)
	ajExit();
    else
    {
	ajWarn("No matches found.");
	ajExitBad();
    }

    return 0;
}




/* @funcstatic notseq_readfile ************************************************
**
** If the list of names starts with a '@', open that file, read in
** the list of names and replaces the input string with the names
**
** @param [r] pattern [AjPStr*] names to search for or '@file'
** @return [void]
** @@
******************************************************************************/

static void notseq_readfile(AjPStr *pattern)
{
    AjPFile file = NULL;
    AjPStr line;
    char *p = NULL;

    if(ajStrFindC(*pattern, "@") == 0)
    {
        ajStrTrimC(pattern, "@");       /* remove the @ */
        file = ajFileNewIn(*pattern);
        if(file == NULL)
            ajFatal("Cannot open the file of sequence names: '%S'", pattern);

        /* blank off the file name and replace with the sequence names */
        ajStrClear(pattern);
        line = ajStrNew();
        while(ajFileReadLine(file, &line))
        {
            p = ajStrStr(line);

            if(!*p || *p == '#' || *p == '!')
		continue;

            ajStrApp(pattern, line);
            ajStrAppC(pattern, ",");
        }
        ajStrDel(&line);

        ajFileClose(&file);
    }

    return;
}

/* @source coderet application
**
** Retrieves CDS, mRNA and translations from feature tables
**
** @author Copyright (C) Alan Bleasby (ableasby@hgmp.mrc.ac.uk)
** Last modified by Jon Ison Thu Jun 29 08:26:08 BST 2006
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




static void coderet_put_seq(const AjPSeq seq, const AjPStr strseq,
			    ajint n, const char *name,
			    ajint type, AjPSeqout seqout);




/* @prog coderet **************************************************************
**
** Extract CDS, mRNA and translations from feature tables
**
******************************************************************************/

int main(int argc, char **argv)
{
    AjPSeqall seqall      = NULL;
    AjPSeq seq            = NULL;
    AjPSeqout seqoutcds   = NULL;
    AjPSeqout seqoutmrna  = NULL;
    AjPSeqout seqoutprot  = NULL;
    AjPFile logf     = NULL;

    ajint ncds  = 0;
    ajint nmrna = 0;
    ajint ntran = 0;
    ajint i =0;

    AjPStr cds  = NULL;
    AjPStr mrna = NULL;
    AjPStr usa  = NULL;

    AjBool ret = ajFalse;
    AjPStr *cdslines  = NULL;
    AjPStr *mrnalines = NULL;
    AjPStr *tranlines = NULL;

    embInit("coderet",argc,argv);

    seqall  = ajAcdGetSeqall("seqall");

    seqoutcds  = ajAcdGetSeqoutall("cdsoutseq");
    seqoutmrna = ajAcdGetSeqoutall("mrnaoutseq");
    seqoutprot = ajAcdGetSeqoutall("translationoutseq");
    logf = ajAcdGetOutfile("outfile");

    cds  = ajStrNew();
    mrna = ajStrNew();
    usa  = ajStrNew();


    /*
    **  Must get this so that embedded references in the same database
    **  can be resolved
    */
    ajStrAssignS(&usa,ajSeqallGetUsa(seqall));

    if(seqoutcds)
	ajFmtPrintF(logf, "   CDS");

    if(seqoutmrna)
	ajFmtPrintF(logf, "  mRNA");

    if(seqoutprot)
	ajFmtPrintF(logf, " Trans");

    ajFmtPrintF(logf, " Total Sequence\n");
    if(seqoutcds)
	ajFmtPrintF(logf, " =====");

    if(seqoutmrna)
	ajFmtPrintF(logf, " =====");

    if(seqoutprot)
	ajFmtPrintF(logf, " =====");

    ajFmtPrintF(logf, " ===== ========\n");

    while(ajSeqallNext(seqall,&seq))
    {
	if(seqoutcds)
	{
	    ncds = ajFeatGetLocs(seq->TextPtr, &cdslines, "CDS");

	    for(i=0;i<ncds;++i)
	    {
		ret = ajFeatLocToSeq(ajSeqStr(seq),cdslines[i],&cds,usa);
		if(!ret)
		{
		    ajWarn("Cannot extract %s\n",ajSeqName(seq));
		    continue;
		}
		coderet_put_seq(seq,cds,i,"cds",0,seqoutcds);
		ajStrDel(&cdslines[i]);
	    }
	    if(ncds)
		AJFREE(cdslines);
	    ajFmtPrintF(logf, "%6d", ncds);
	}

	if(seqoutmrna)
	{
	    nmrna = ajFeatGetLocs(seq->TextPtr, &mrnalines, "mRNA");

	    for(i=0;i<nmrna;++i)
	    {
		ret = ajFeatLocToSeq(ajSeqStr(seq),mrnalines[i],&mrna,usa);
		if(!ret)
		{
		    ajWarn("Cannot extract %s",ajSeqName(seq));
		    continue;
		}
		coderet_put_seq(seq,mrna,i,"mrna",0,seqoutmrna);
		ajStrDel(&mrnalines[i]);
	    }

	    if(nmrna)
		AJFREE(mrnalines);
	    ajFmtPrintF(logf, "%6d", nmrna);
	}


	if(seqoutprot)
	{
	    ntran = ajFeatGetTrans(seq->TextPtr, &tranlines);

	    for(i=0;i<ntran;++i)
	    {
		coderet_put_seq(seq,tranlines[i],i,"pro",1,seqoutprot);
		ajStrDel(&tranlines[i]);
	    }

	    if(ntran)
		AJFREE(tranlines);
	    ajFmtPrintF(logf, "%6d", ntran);
	}
	ajFmtPrintF(logf, "%6d %s\n", ncds+nmrna+ntran, ajSeqName(seq));
    }


    if(seqoutcds)
	ajSeqWriteClose(seqoutcds);
    if(seqoutmrna)
	ajSeqWriteClose(seqoutmrna);
    if(seqoutprot)
	ajSeqWriteClose(seqoutprot);


    ajStrDel(&cds);
    ajStrDel(&mrna);
    ajStrDel(&usa);
    ajSeqallDel(&seqall);
    ajSeqDel(&seq);
    ajSeqoutDel(&seqoutcds);
    ajSeqoutDel(&seqoutmrna);
    ajSeqoutDel(&seqoutprot);
    ajFileClose(&logf);

    ajExit();

    return 0;
}




/* @funcstatic coderet_put_seq ************************************************
**
** Undocumented.
**
** @param [r] seq [const AjPSeq] Undocumented
** @param [r] strseq [const AjPStr] Undocumented
** @param [r] n [ajint] Undocumented
** @param [r] name [const char*] Undocumented
** @param [r] type [ajint] Undocumented
** @param [u] seqout [AjPSeqout] Undocumented
** @@
******************************************************************************/

static void coderet_put_seq(const AjPSeq seq, const AjPStr strseq,
			    ajint n, const char *name,
			    ajint type, AjPSeqout seqout)
{
    AjPSeq nseq = NULL;
    AjPStr fn   = NULL;

    fn = ajStrNew();


    ajFmtPrintS(&fn,"%S_%s_%d",ajSeqGetAcc(seq),name,n+1);
    ajStrFmtLower(&fn);

    nseq = ajSeqNewL(ajStrGetLen(strseq));
    ajSeqAssignNameS(nseq, fn);
    ajSeqAssignEntryS(nseq, fn);

    if(!type)
	ajSeqSetNuc(nseq);
    else
	ajSeqSetProt(nseq);

    ajSeqAssignSeqS(nseq,strseq);


    ajSeqWrite(seqout,nseq);


    ajSeqDel(&nseq);
    ajStrDel(&fn);

    return;
}

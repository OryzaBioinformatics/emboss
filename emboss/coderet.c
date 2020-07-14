/* @source coderet application
**
** Retrieves CDS, mRNA and translations from feature tables
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

static void coderet_put_seq(AjPSeq seq, AjPStr strseq, ajint n, char *name,
			    ajint type, AjPSeqout seqout);




/* @prog coderet **************************************************************
**
** Extract CDS, mRNA and translations from feature tables
**
******************************************************************************/

int main(int argc, char **argv)
{
    AjPSeqall seqall=NULL;
    AjPSeq seq=NULL;
    AjPSeqout seqout=NULL;

    ajint ncds=0;
    ajint nmrna=0;
    ajint ntran=0;
    ajint i=0;

    AjPStr cds=NULL;
    AjPStr mrna=NULL;
    AjPStr usa=NULL;

    AjBool ret=ajFalse;
    AjPStr *cdslines=NULL;
    AjPStr *mrnalines=NULL;
    AjPStr *tranlines=NULL;
    AjBool docds=ajFalse;
    AjBool domrna=ajFalse;
    AjBool dotran=ajFalse;


    embInit("coderet",argc,argv);

    seqall  = ajAcdGetSeqall("seqall");

    domrna = ajAcdGetBool("mrna");
    docds  = ajAcdGetBool("cds");
    dotran = ajAcdGetBool("translation");

    seqout = ajAcdGetSeqout("outseq");

    cds  = ajStrNew();
    mrna = ajStrNew();
    usa  = ajStrNew();


    /*
     *  Must get this so that embedded references in the same database
     *  can be resolved
     */
    ajStrAssS(&usa,ajSeqallGetUsa(seqall));

    while(ajSeqallNext(seqall,&seq))
    {
	if(docds)
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
		coderet_put_seq(seq,cds,i,"cds",0,seqout);
		ajStrDel(&cdslines[i]);
	    }
	    if(ncds)
		AJFREE(cdslines);
	}

	if(domrna)
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
		coderet_put_seq(seq,mrna,i,"mrna",0,seqout);
		ajStrDel(&mrnalines[i]);
	    }

	    if(nmrna)
		AJFREE(mrnalines);
	}


	if(dotran)
	{
	    ntran = ajFeatGetTrans(seq->TextPtr, &tranlines);

	    for(i=0;i<ntran;++i)
	    {
		coderet_put_seq(seq,tranlines[i],i,"pro",1,seqout);
		ajStrDel(&tranlines[i]);
	    }

	    if(nmrna)
		AJFREE(tranlines);
	}
    }


    ajSeqWriteClose(seqout);


    ajStrDel(&cds);
    ajStrDel(&mrna);
    ajStrDel(&usa);

    ajExit();
    return 0;
}




/* @funcstatic coderet_put_seq ************************************************
**
** Undocumented.
**
** @param [?] seq [AjPSeq] Undocumented
** @param [?] strseq [AjPStr] Undocumented
** @param [?] n [ajint] Undocumented
** @param [?] name [char*] Undocumented
** @param [?] type [ajint] Undocumented
** @param [?] seqout [AjPSeqout] Undocumented
** @@
******************************************************************************/

static void coderet_put_seq(AjPSeq seq, AjPStr strseq, ajint n, char *name,
			    ajint type, AjPSeqout seqout)
{
    AjPSeq nseq=NULL;
    AjPStr fn=NULL;

    fn = ajStrNew();


    ajFmtPrintS(&fn,"%S_%s_%d",ajSeqGetAcc(seq),name,n+1);
    ajStrToLower(&fn);

    nseq = ajSeqNewL(ajStrLen(strseq));
    ajSeqAssName(nseq, fn);
    ajSeqAssEntry(nseq, fn);

    if(!type)
	ajSeqSetNuc(nseq);
    else
	ajSeqSetProt(nseq);

    ajSeqReplace(nseq,strseq);


    ajSeqWrite (seqout,nseq);


    ajSeqDel(&nseq);
    ajStrDel(&fn);


    return;
}

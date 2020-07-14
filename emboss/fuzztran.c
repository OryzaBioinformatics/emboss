/* @source fuzztran application
**
** Finds fuzzy protein patterns in nucleic acid sequences
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
#include "stdlib.h"




static void fuzztran_save_hits(AjPList *l, ajint hits, ajint fnum,
			       AjPStr pro,
			       AjPFeattable* ptab, AjPSeq seq);




/* @prog fuzztran *************************************************************
**
** Protein pattern search after translation
**
******************************************************************************/

int main(int argc, char **argv)
{
    AjPSeqall seqall;
    AjPSeq seq;
    AjPReport report = NULL;
    AjPFeattable tab = NULL;
    AjPStr pattern   = NULL;
    AjPStr opattern  = NULL;
    AjPStr seqname   = NULL;
    AjPStr text      = NULL;

    AjPList l;

    AjPStr *lframe;
    AjPStr *lgcode;
    AjPTrn ttable;
    AjPStr frame;
    ajint  table;
    AjPStr pro = NULL;
    ajint frameno;


    ajint plen;
    ajint mismatch;

    AjBool amino;
    AjBool carboxyl;

    ajint type = 0;
    ajint *buf = NULL;
    ajint hits = 0;
    ajint m;
    ajint i;
    ajint begin;
    ajint end;

    EmbOPatBYPNode off[AJALPHA];

    ajuint *sotable = NULL;
    ajuint solimit;

    AjPStr regexp = NULL;
    ajint **skipm = NULL;


    AjPStr tmpstr = NULL;
    void  *tidy   = NULL;

    embInit("fuzztran", argc, argv);

    seqall   = ajAcdGetSeqall("sequence");
    pattern  = ajAcdGetString("pattern");
    report   = ajAcdGetReport("outfile");
    mismatch = ajAcdGetInt("mismatch");
    lgcode   = ajAcdGetList("table");
    lframe   = ajAcdGetList("frame");

    ajFmtPrintAppS(&tmpstr, "Pattern: %S\n", pattern);
    ajFmtPrintAppS(&tmpstr, "Mismatch: %d\n", mismatch);
    ajFmtPrintAppS(&tmpstr, "TransTable: %S\n", ajAcdValue("table"));
    ajFmtPrintAppS(&tmpstr, "Frames: %S\n", ajAcdValue("frame"));
    ajReportSetHeader(report, tmpstr);


    ajStrTrimEndC(&pattern," .\t\n");

    seqname  = ajStrNew();
    opattern = ajStrNew();

    frame = lframe[0];
    ajStrToInt(lgcode[0],&table);
    ttable = ajTrnNewI(table);


    plen = ajStrLen(pattern);
    ajStrAssC(&opattern,ajStrStr(pattern));

    if(!(type=embPatGetType(&pattern,mismatch,1,&m,&amino,&carboxyl)))
	ajFatal("Illegal pattern");
    embPatCompile(type,pattern,opattern,&plen,&buf,off,&sotable,&solimit,
		  &m,&regexp,&skipm,mismatch);


    text = ajStrNew();
    pro  = ajStrNew();


    while(ajSeqallNext(seqall,&seq))
    {
	l = ajListNew();
	ajStrAssC(&seqname,ajSeqName(seq));
	begin = ajSeqallBegin(seqall);
	end   = ajSeqallEnd(seqall);
	ajStrAssSubC(&text,ajSeqChar(seq),begin-1,end-1);
	ajStrToUpper(&text);

	if(!ajStrCmpC(frame,"F"))
	{
	    ajTrnStrFrame(ttable,text,1,&pro);
	    embPatFuzzSearch(type,begin,pattern,opattern,seqname,pro,&l,
			     plen,mismatch,amino,carboxyl,buf,off,sotable,
			     solimit,regexp,skipm,&hits,m,&tidy);
	    if(hits)
		fuzztran_save_hits(&l,hits,1, pro, &tab, seq);

	    ajStrAssC(&pro,"");

	    ajTrnStrFrame(ttable,text,2,&pro);
	    embPatFuzzSearch(type,begin,pattern,opattern,seqname,pro,&l,
			     plen,mismatch,amino,carboxyl,buf,off,sotable,
			     solimit,regexp,skipm,&hits,m,&tidy);
	    if(hits)
		fuzztran_save_hits(&l,hits,2, pro, &tab, seq);

	    ajStrAssC(&pro,"");

	    ajTrnStrFrame(ttable,text,3,&pro);
	    embPatFuzzSearch(type,begin,pattern,opattern,seqname,pro,&l,
			     plen,mismatch,amino,carboxyl,buf,off,sotable,
			     solimit,regexp,skipm,&hits,m,&tidy);
	    if(hits)
		fuzztran_save_hits(&l,hits,3, pro, &tab, seq);

	    ajStrAssC(&pro,"");
	}
	else if(!ajStrCmpC(frame,"R"))
	{
	    ajTrnStrFrame(ttable,text,-1,&pro);
	    embPatFuzzSearch(type,begin,pattern,opattern,seqname,pro,&l,
			     plen,mismatch,amino,carboxyl,buf,off,sotable,
			     solimit,regexp,skipm,&hits,m,&tidy);
	    if(hits)
		fuzztran_save_hits(&l,hits, -1, pro, &tab, seq);

	    ajStrAssC(&pro,"");

	    ajTrnStrFrame(ttable,text,-2,&pro);
	    embPatFuzzSearch(type,begin,pattern,opattern,seqname,pro,&l,
			     plen,mismatch,amino,carboxyl,buf,off,sotable,
			     solimit,regexp,skipm,&hits,m,&tidy);
	    if(hits)
		fuzztran_save_hits(&l,hits,-2, pro, &tab, seq);

	    ajStrAssC(&pro,"");

	    ajTrnStrFrame(ttable,text,-3,&pro);
	    embPatFuzzSearch(type,begin,pattern,opattern,seqname,pro,&l,
			     plen,mismatch,amino,carboxyl,buf,off,sotable,
			     solimit,regexp,skipm,&hits,m,&tidy);
	    if(hits)
		fuzztran_save_hits(&l,hits,-3, pro, &tab, seq);

	    ajStrAssC(&pro,"");
	}
	else if(!ajStrCmpC(frame,"6"))
	{
	    ajTrnStrFrame(ttable,text,1,&pro);
	    embPatFuzzSearch(type,begin,pattern,opattern,seqname,pro,&l,
			     plen,mismatch,amino,carboxyl,buf,off,sotable,
			     solimit,regexp,skipm,&hits,m,&tidy);
	    if(hits)
		fuzztran_save_hits(&l,hits, 1, pro, &tab, seq);

	    ajStrAssC(&pro,"");

	    ajTrnStrFrame(ttable,text,2,&pro);
	    embPatFuzzSearch(type,begin,pattern,opattern,seqname,pro,&l,
			     plen,mismatch,amino,carboxyl,buf,off,sotable,
			     solimit,regexp,skipm,&hits,m,&tidy);
	    if(hits)
		fuzztran_save_hits(&l,hits, 2, pro, &tab, seq);

	    ajStrAssC(&pro,"");

	    ajTrnStrFrame(ttable,text,3,&pro);
	    embPatFuzzSearch(type,begin,pattern,opattern,seqname,pro,&l,
			     plen,mismatch,amino,carboxyl,buf,off,sotable,
			     solimit,regexp,skipm,&hits,m,&tidy);
	    if(hits)
		fuzztran_save_hits(&l,hits,3, pro, &tab, seq);

	    ajStrAssC(&pro,"");

	    ajTrnStrFrame(ttable,text,-1,&pro);
	    embPatFuzzSearch(type,begin,pattern,opattern,seqname,pro,&l,
			     plen,mismatch,amino,carboxyl,buf,off,sotable,
			     solimit,regexp,skipm,&hits,m,&tidy);
	    if(hits)
		fuzztran_save_hits(&l,hits,-1, pro, &tab, seq);

	    ajStrAssC(&pro,"");

	    ajTrnStrFrame(ttable,text,-2,&pro);
	    embPatFuzzSearch(type,begin,pattern,opattern,seqname,pro,&l,
			     plen,mismatch,amino,carboxyl,buf,off,sotable,
			     solimit,regexp,skipm,&hits,m,&tidy);
	    if(hits)
		fuzztran_save_hits(&l,hits, -2, pro, &tab, seq);

	    ajStrAssC(&pro,"");

	    ajTrnStrFrame(ttable,text,-3,&pro);
	    embPatFuzzSearch(type,begin,pattern,opattern,seqname,pro,&l,
			     plen,mismatch,amino,carboxyl,buf,off,sotable,
			     solimit,regexp,skipm,&hits,m,&tidy);
	    if(hits)
		fuzztran_save_hits(&l,hits, -3, pro, &tab, seq);

	    ajStrAssC(&pro,"");
	}
	else
	{
	    ajStrToInt(frame,&frameno);
	    ajTrnStrFrame(ttable,text,frameno,&pro);
	    embPatFuzzSearch(type,begin,pattern,opattern,seqname,pro,&l,
			     plen,mismatch,amino,carboxyl,buf,off,sotable,
			     solimit,regexp,skipm,&hits,m,&tidy);
	    if(hits)
		fuzztran_save_hits(&l,hits,frameno, pro, &tab, seq);

	    ajStrAssC(&pro,"");
	}

	if(ajFeattableSize(tab))
	{
	  ajReportWrite(report, tab, seq);
	  ajFeattableDel(&tab);
	}

	ajListDel(&l);
    }



    if(type==6)
	for(i=0;i<m;++i)
	    AJFREE(skipm[i]);

    if(tidy)
	AJFREE(tidy);

    ajStrDel(&pro);
    ajStrDel(&text);
    ajStrDel(&pattern);
    ajStrDel(&seqname);
    ajSeqDel(&seq);

    ajReportClose(report);
    ajReportDel(&report);
    ajExit();

    return 0;
}




/* @funcstatic fuzztran_save_hits *********************************************
**
** Save the hits in a feature table for later reporting.
**
** @param [?] l [AjPList*] List of hits stored as EmbPMatch objects
** @param [?] hits [ajint] Number of hits
** @param [?] fnum [ajint] Frame number 1, 2, 3, -1, -2 or -3.
** @param [r] pro [AjPStr] Protein translation
** @param [?] ptab [AjPFeattable*] Feature table (created if first use)
** @param [?] seq [AjPSeq] Sequence
** @@
******************************************************************************/


static void fuzztran_save_hits(AjPList *l, ajint hits, ajint fnum,
			       AjPStr pro,
			       AjPFeattable* ptab, AjPSeq seq)
{
    ajint i;
    EmbPMatMatch m;
    ajint ff;
    AjBool forward;
    ajint slen;
    ajint npos;
    ajint nlast;
    AjPFeattable tab;
    char strand = '+';
    ajint begin;
    ajint end;
    AjPFeature gf = NULL;
    static AjPStr fthit;
    static AjPStr s = NULL;
    static AjPStr t = NULL;

    if(!fthit)
	ajStrAssC(&fthit, "hit");

    if(!*ptab)
	*ptab = ajFeattableNewSeq(seq);

    tab = *ptab;

    begin = ajSeqBegin(seq);
    end   = ajSeqEnd(seq);

    forward=ajTrue;
    if(fnum<0)
    {
	forward=ajFalse;
	strand = '-';
    }

    slen = end-begin+1;

    s = ajStrNew();
    ff = abs(fnum);
    if(ff>3)
    {
	ff -= 3;
	forward = ajFalse;
    }

    ff %= 3;
    if(ff)
	ff -= 3;

    ajListReverse(*l);

    for(i=0;i<hits;++i)
    {
	ajListPop(*l,(void **)&m);
	if(forward)
	{
	    npos  = (m->start*3) + ff + (begin-1);
	    nlast = npos + m->len*3 - 1;
	}
	else
	{
	    nlast = (slen/3)*3  - (m->start*3) - (fnum-1) + (begin);
	    npos  = nlast - m->len*3 + 1;
	}

        gf = ajFeatNew(tab, NULL, fthit, npos, nlast,
		       (float) (m->len - m->mm), strand, fnum);

	if(m->mm)
	{
	    ajFmtPrintS(&s, "*mismatch %d", m->mm);
	    ajFeatTagAdd(gf, NULL, s);
	}

	ajFmtPrintS(&s, "*frame %d", fnum);
	ajFeatTagAdd(gf, NULL, s);

	ajFmtPrintS(&s, "*start %d", m->start);
	ajFeatTagAdd(gf, NULL, s);

	ajFmtPrintS(&s, "*end %d", m->start + m->len - 1);
	ajFeatTagAdd(gf, NULL, s);

	ajStrAssSub(&t,pro,m->start-1,m->start+m->len-2);
	ajFmtPrintS(&s, "*translation %S", t);
	ajFeatTagAdd(gf, NULL, s);

	embMatMatchDel(&m);
    }


    ajStrDel(&s);

    return;
}

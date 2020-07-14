/* @source fuzznuc application
**
** Finds fuzzy patterns in nucleic acid sequences
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




static void fuzznuc_report_hits(AjPList *l, ajint hits,
				AjBool sc, ajint thits,
				AjPReport report,
				AjPFeattable tab, const AjPSeq seq);




/* @prog fuzznuc **************************************************************
**
** Nucleic acid pattern search
**
******************************************************************************/

int main(int argc, char **argv)
{
    AjPSeqall seqall;
    AjPSeq seq;
    AjPFeattable tab = NULL;
    AjPReport report = NULL;
    AjPStr pattern   = NULL;
    AjPStr opattern  = NULL;
    AjPStr seqname   = NULL;
    AjPStr text      = NULL;
    AjPList l;

    ajint plen;
    ajint mismatch;
    ajint thits = 0;

    AjBool amino;
    AjBool carboxyl;
    AjBool sc;

    ajint type = 0;
    ajint hits = 0;
    ajint m;
    ajint i;
    ajint begin;
    ajint end;
    ajint adj;

    EmbOPatBYPNode off[AJALPHA];
    ajuint *sotable = NULL;
    ajuint solimit;
    AjPStr regexp = NULL;
    ajint **skipm = NULL;
    ajint *buf    = NULL;
    AjPStr tmpstr = NULL;
    void *tidy    = NULL;

    embInit("fuzznuc", argc, argv);

    seqall   = ajAcdGetSeqall("sequence");
    report   = ajAcdGetReport("outfile");
    pattern  = ajAcdGetString("pattern");
    mismatch = ajAcdGetInt("mismatch");
    sc       = ajAcdGetBool("complement");

    ajFmtPrintAppS(&tmpstr, "Pattern: %S\n", pattern);
    ajFmtPrintAppS(&tmpstr, "Mismatch: %d\n", mismatch);
    ajFmtPrintAppS(&tmpstr, "Complement: %B\n", sc);
    ajReportSetHeader(report, tmpstr);

    ajStrTrimEndC(&pattern," .\t\n");

    seqname = ajStrNew();
    opattern=ajStrNew();

    /* Copy original pattern regexps */
    ajStrAssC(&opattern,ajStrStr(pattern));

    if(!(type=embPatGetType(opattern,&pattern,mismatch,0,&m,&amino,&carboxyl)))
	ajFatal("Illegal pattern");
    embPatCompile(type,pattern,&plen,&buf,off,&sotable,&solimit,&m,
		  &regexp,&skipm,mismatch);

    text = ajStrNew();


    while(ajSeqallNext(seqall,&seq))
    {
	l = ajListNew();
	ajStrAssC(&seqname,ajSeqName(seq));
	begin = ajSeqBegin(seq);
	end   = ajSeqEnd(seq);
	ajStrAssSubC(&text,ajSeqChar(seq),begin-1,end-1);
	ajStrToUpper(&text);
	adj = begin+end+1;

	embPatFuzzSearch(type,begin,pattern,seqname,text,l,
			 plen,mismatch,amino,carboxyl,buf,off,sotable,
			 solimit,regexp,skipm,&hits,m,&tidy);
	if(sc)
	{
	    ajSeqReverseStr(&text);
	    embPatFuzzSearch(type,begin,pattern,seqname,text,l,
			     plen,mismatch,amino,carboxyl,buf,off,sotable,
			     solimit,regexp,skipm,&thits,m,&tidy);
	    ajSeqReverseStr(&text);
	}



	if(hits || (thits&&sc))
	{
	    tab = ajFeattableNewDna(seqname);
	    fuzznuc_report_hits(&l,hits,sc,thits,
				report, tab, seq);
	    ajFeattableDel(&tab);
	}



	ajListDel(&l);
    }



    if(type==6)
	for(i=0;i<m;++i) AJFREE(skipm[i]);

    if(tidy)
	AJFREE(tidy);

    ajStrDel(&pattern);
    ajStrDel(&seqname);
    ajSeqDel(&seq);

    ajReportClose(report);
    ajReportDel(&report);

    ajExit();
    return 0;
}




/* @funcstatic fuzznuc_report_hits ********************************************
**
** Undocumented.
**
** @param [u] l [AjPList*] Undocumented
** @param [r] hits [ajint] Undocumented
** @param [r] sc [AjBool] Undocumented
** @param [r] thits [ajint] Undocumented
** @param [u] report [AjPReport] Report object
** @param [u] tab [AjPFeattable] Feature table
** @param [r] seq [const AjPSeq] Sequence
** @@
******************************************************************************/


static void fuzznuc_report_hits(AjPList *l, ajint hits,
				AjBool sc, ajint thits,
				AjPReport report,
			        AjPFeattable tab, const AjPSeq seq)
{
    ajint i;
    EmbPMatMatch m;
    AjPStr s;
    AjPFeature gf = NULL;
    static AjPStr fthit;
    ajint begin;
    ajint end;
    ajint adj;

    begin = ajSeqBegin(seq) - 1;
    end   = ajSeqEnd(seq) - 1;
    adj   =  ajSeqBegin(seq) + ajSeqEnd(seq) + 1;

    if(!fthit)
      ajStrAssC(&fthit, "hit");

    s = ajStrNew();

    ajListReverse(*l);

    for(i=0;i<hits;++i)
    {
	ajListPop(*l,(void **)&m);
        gf = ajFeatNew(tab, NULL, fthit,
		       m->start,
		       m->start + m->len - 1,
		       (float) (m->len - m->mm), '+', 0);

	if(m->mm)
	{
	    ajFmtPrintS(&s, "*mismatch %d", m->mm);
	    ajFeatTagAdd(gf, NULL, s);
	}

	embMatMatchDel(&m);
    }

    if(sc)
    {
	ajListReverse(*l);
	for(i=0;i<thits;++i)
	{
	    ajListPop(*l,(void **)&m);
	    gf = ajFeatNew(tab, NULL, fthit,
			    adj - m->start - m->len,
			    adj - m->start - 1,
			    (float) (m->len - m->mm), '-', 0);

	    if(m->mm)
	    {
		ajFmtPrintS(&s, "*mismatch %d", m->mm);
		ajFeatTagAdd(gf, NULL, s);
	    }

	    embMatMatchDel(&m);
	}
    }

    ajReportWrite(report, tab, seq);

    ajStrDel(&s);

    return;
}

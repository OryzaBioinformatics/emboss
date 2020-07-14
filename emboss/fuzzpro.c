/* @source fuzzpro application
**
** Finds fuzzy patterns in proteins
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




static void fuzzpro_report_hits(AjPList *l, ajint hits,
				AjPReport report,
				AjPFeattable tab, AjPSeq seq);




/* @prog fuzzpro **************************************************************
**
** Protein pattern search
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

    AjBool amino;
    AjBool carboxyl;
    ajint    type = 0;
    ajint    *buf = NULL;
    ajint    hits = 0;
    ajint    m;
    ajint    i;
    ajint    end;
    ajint    begin;

    EmbOPatBYPNode off[AJALPHA];

    ajuint *sotable = NULL;
    ajuint solimit;

    AjPStr regexp = NULL;

    ajint **skipm = NULL;

    AjPStr tmpstr = NULL;
    void   *tidy  = NULL;


    embInit("fuzzpro", argc, argv);

    seqall   = ajAcdGetSeqall("sequence");
    report   = ajAcdGetReport("outfile");
    pattern  = ajAcdGetString("pattern");
    mismatch = ajAcdGetInt("mismatch");

    ajFmtPrintAppS(&tmpstr, "Pattern: %S\n", pattern);
    ajFmtPrintAppS(&tmpstr, "Mismatch: %d\n", mismatch);
    ajReportSetHeader(report, tmpstr);


    ajStrTrimEndC(&pattern," .\t\n");

    seqname  = ajStrNew();
    opattern = ajStrNew();

    plen = ajStrLen(pattern);

    ajStrAssC(&opattern,ajStrStr(pattern));
    if(!(type=embPatGetType(&pattern,mismatch,1,&m,&amino,&carboxyl)))
	ajFatal("Illegal pattern");
    embPatCompile(type,pattern,opattern,&plen,&buf,off,&sotable,&solimit,&m,
		  &regexp,&skipm,mismatch);

    text = ajStrNew();


    while(ajSeqallNext(seqall,&seq))
    {
	l = ajListNew();
	ajStrAssC(&seqname,ajSeqName(seq));
	begin = ajSeqallBegin(seqall);
	end   = ajSeqallEnd(seqall);
	ajStrAssSubC(&text,ajSeqChar(seq),begin-1,end-1);
	ajStrToUpper(&text);

	embPatFuzzSearch(type,begin,pattern,opattern,seqname,text,&l,
			 plen,mismatch,amino,carboxyl,buf,off,sotable,
			 solimit,regexp,skipm,&hits,m,&tidy);

	if(hits)
	{
	    tab = ajFeattableNewProt(seqname);
	    fuzzpro_report_hits(&l,hits,report, tab, seq);
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




/* @funcstatic fuzzpro_report_hits ********************************************
**
** Undocumented.
**
** @param [?] l [AjPList*] Undocumented
** @param [?] hits [ajint] Undocumented
** @param [?] report [AjPReport] Report object
** @param [?] tab [AjPFeattable] Feature table
** @param [?] seq [AjPSeq] Sequence
** @@
******************************************************************************/

static void fuzzpro_report_hits(AjPList *l, ajint hits,
				AjPReport report,
				AjPFeattable tab, AjPSeq seq)
{
    ajint i;
    EmbPMatMatch m;
    AjPStr s;
    AjPFeature gf = NULL;
    static AjPStr fthit;
    ajint begin;

    begin = ajSeqBegin(seq) - 1;

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
		       (float) (m->len - m->mm), '?', 0);

	if(m->mm)
	{
	    ajFmtPrintS(&s, "*mismatch %d", m->mm);
	    ajFeatTagAdd(gf, NULL, s);
	}

	embMatMatchDel(&m);
    }

    ajReportWrite(report, tab, seq);

    ajStrDel(&s);

    return;
}

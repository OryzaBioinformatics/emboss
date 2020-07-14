/* @source digest application
**
** Calculate protein proteolytic (and CNBr) digest fragments
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
#include <strings.h>




static void digest_report_hits(AjPReport report, const AjPSeq seq,
			       AjPFeattable TabRpt,
			       AjPList l, ajint be, const char *s);
static void digest_print_hits(AjPList l, AjPFile outf, ajint be,
			      const char *s);




/* @prog digest ***************************************************************
**
** Protein proteolytic enzyme or reagent cleavage digest
**
******************************************************************************/

int main(int argc, char **argv)
{
    AjPSeq  a;
    AjPStr  substr;
    AjPStr  rname;
    ajint be;
    ajint en;
    ajint len;

    AjBool unfavoured;
    AjBool overlap;
    AjBool allpartials;
    AjPStr *menu;
    ajint  n;

    AjPFile  outf = NULL;
    AjPReport report    = NULL;
    AjPFeattable TabRpt = NULL;
    AjPStr tmpStr = NULL;
    AjPList  l;
    AjPList  pa;
    AjPFile mfptr   = NULL;

    ajint     ncomp;
    ajint     npart;


    embInit("digest", argc, argv);

    a           = ajAcdGetSeq("sequence");
    menu        = ajAcdGetList("menu");
    unfavoured  = ajAcdGetBool("unfavoured");
    overlap     = ajAcdGetBool("overlap");
    allpartials = ajAcdGetBool("allpartials");
    report      = ajAcdGetReport("outfile");
    mfptr       = ajAcdGetDatafile("aadata");

    /* obsolete. Can be uncommented in acd file and here to reuse */

    /* outf      = ajAcdGetOutfile("originalfile"); */

    sscanf(ajStrStr(*menu),"%d",&n);
    --n;

    substr = ajStrNew();
    be     = ajSeqBegin(a);
    en     = ajSeqEnd(a);
    ajStrAssSubC(&substr,ajSeqChar(a),be-1,en-1);
    len = en-be+1;
    
    l     = ajListNew();
    pa    = ajListNew();
    rname = ajStrNew();
    
    TabRpt = ajFeattableNewSeq(a);
    
    embPropAminoRead(mfptr);
    
    embPropCalcFragments(ajStrStr(substr),n,be,&l,&pa,unfavoured,overlap,
			 allpartials,&ncomp,&npart,&rname);
    
    
    if(outf)
	ajFmtPrintF(outf,"DIGEST of %s from %d to %d Molwt=%10.3f\n\n",
		    ajSeqName(a),be,en,embPropCalcMolwt(ajSeqChar(a),0,len-1));
    if(!ncomp)
    {
	if(outf)
	    ajFmtPrintF(outf,"Is not proteolytically digested using %s\n",
			ajStrStr(rname));
    }
    else
    {
	if(outf)
	{
	    ajFmtPrintF(outf,"Complete digestion with %s "
			"yields %d fragments:\n",
			ajStrStr(rname),ncomp);
	    digest_print_hits(l,outf,be,ajStrStr(substr));
	}
	ajFmtPrintS(&tmpStr,
		    "Complete digestion with %S yields %d fragments",
		    rname,ncomp);
	ajReportSetHeader(report, tmpStr);
	digest_report_hits(report, a, TabRpt,l,be, ajStrStr(substr));
	ajReportWrite(report, TabRpt, a);
	ajFeattableClear(TabRpt);
    }
    
    if(overlap && !allpartials && npart)
    {
	if(outf)
	{
	    ajFmtPrintF(outf,"\n\nPartial digest with %s yields %d extras.\n",
			ajStrStr(rname),npart);
	    ajFmtPrintF(outf,"Only overlapping partials shown:\n");
	    digest_print_hits(pa,outf,be,ajStrStr(substr));
	}
	ajFmtPrintS(&tmpStr,
		    "\n\nPartial digest with %S yields %d extras.\n",
		    rname,npart);
	ajFmtPrintAppS(&tmpStr,"Only overlapping partials shown:\n");
	ajReportSetHeader(report, tmpStr);
	digest_report_hits(report, a, TabRpt, pa,be,ajStrStr(substr));
	ajReportWrite(report, TabRpt, a);
	ajFeattableClear(TabRpt);
    }
    
    if(allpartials && npart)
    {
	if(outf)
	{
	    ajFmtPrintF(outf,"\n\nPartial digest with %s yields %d extras.\n",
			ajStrStr(rname),npart);
	    ajFmtPrintF(outf,"All partials shown:\n");
	    digest_print_hits(pa,outf,be,ajStrStr(substr));
	}
	ajFmtPrintS(&tmpStr,
		    "\n\nPartial digest with %S yields %d extras.\n",
		    rname,npart);
	ajFmtPrintAppS(&tmpStr,"All partials shown:\n");
	ajReportSetHeader(report, tmpStr);
	digest_report_hits(report, a, TabRpt, pa,be, ajStrStr(substr));
	ajReportWrite(report, TabRpt, a);
	ajFeattableClear(TabRpt);
    }
    
    
    ajFeattableDel(&TabRpt);
    
    ajSeqDel(&a);
    ajStrDel(&rname);
    ajStrDel(&substr);
    ajListDel(&pa);
    ajListDel(&l);
    
    if(outf)
	ajFileClose(&outf);
    ajFileClose(&mfptr);
    
    ajExit();

    return 0;
}




/* @funcstatic digest_print_hits **********************************************
**
** Undocumented.
**
** @param [u] l [AjPList] Undocumented
** @param [u] outf [AjPFile] Undocumented
** @param [r] be [ajint] Undocumented
** @param [r] s [const char*] Undocumented
** @@
******************************************************************************/



static void digest_print_hits(AjPList l, AjPFile outf, ajint be, const char *s)
{
    EmbPPropFrag fr;
    AjPStr t;
    ajint len;

    t   = ajStrNew();
    len = strlen(s);

    ajFmtPrintF(outf,
		"Start   End     Molwt      Sequence (up to 38 residues)\n");
    while(ajListPop(l,(void **)&fr))
    {
	ajStrAssSubC(&t,s,fr->start,fr->end);
	ajFmtPrintF(outf,"%-8d%-8d%-10.3f ",fr->start+be,fr->end+be,
		    fr->molwt);
	if(fr->start>0)
	    ajFmtPrintF(outf,"(%c) ",*(s+(fr->start+be-1)-1));
	else
	    ajFmtPrintF(outf," () ");

	ajFmtPrintF(outf,"%-.38s ",ajStrStr(t));
	if(fr->end<len-1)
	    ajFmtPrintF(outf,"(%c) ",*(s+(fr->end+be)));
	else
	    ajFmtPrintF(outf," () ");

	if(fr->end-fr->start+1>38)
	    ajFmtPrintF(outf,"...");
	ajFmtPrintF(outf,"\n");
	AJFREE(fr);
    }

    ajStrDel(&t);

    return;
}




/* @funcstatic digest_report_hits *********************************************
**
** Undocumented.
**
** @param [w] report [AjPReport] report
** @param [r] seq [const AjPSeq] sequence object
** @param [u] TabRpt [AjPFeattable] feature table object to store results
** @param [u] l [AjPList] Undocumented
** @param [r] be [ajint] Undocumented
** @param [r] s [const char*] Undocumented
** @@
******************************************************************************/

static void digest_report_hits(AjPReport report, const AjPSeq seq,
			       AjPFeattable TabRpt, AjPList l, ajint be,
			       const char* s)
{
    AjPFeature gf = NULL;
    EmbPPropFrag fr;
    AjPStr t;
    ajint len;
    static AjPStr tmpStr = NULL;

    t   = ajStrNew();
    len = strlen(s);

    while(ajListPop(l,(void **)&fr))
    {
	ajStrAssSubC(&t,s,fr->start,fr->end);
	gf = ajFeatNewII(TabRpt,fr->start+be,fr->end+be);
	ajFmtPrintS(&tmpStr, "*molwt %.3f", fr->molwt);
	ajFeatTagAdd(gf,  NULL, tmpStr);
	if(fr->start>0)
	{
	    ajFmtPrintS(&tmpStr, "*cterm %c", *(s+(fr->start+be-1)-1));
	    ajFeatTagAdd(gf,  NULL, tmpStr);
	}

	if(fr->end<len-1)
	{
	    ajFmtPrintS(&tmpStr, "*nterm %c", *(s+(fr->end+be)));
	    ajFeatTagAdd(gf,  NULL, tmpStr);
	}

	AJFREE(fr);
    }

    ajStrDel(&t);

    return;
}

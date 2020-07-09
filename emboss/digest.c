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

static void digest_print_hits(AjPList l, AjPFile outf, ajint be, char *s);




/* @prog digest ***************************************************************
**
** Protein proteolytic enzyme or reagent cleavage digest
**
******************************************************************************/

int main(int argc, char **argv)
{

    AjPSeq   a;
    AjPStr   substr;
    AjPStr   rname;
    ajint      be;
    ajint      en;
    ajint      len;

    AjBool unfavoured;
    AjBool overlap;
    AjBool allpartials;
    AjPStr *menu;
    ajint    n;
    
    AjPFile  outf;
    AjPList  l;
    AjPList  pa;

    ajint     ncomp;
    ajint     npart;
    
    
    embInit("digest", argc, argv);

    a           = ajAcdGetSeq("sequencea");
    menu        = ajAcdGetList("menu");
    unfavoured  = ajAcdGetBool("unfavoured");
    overlap     = ajAcdGetBool("overlap");
    allpartials = ajAcdGetBool("allpartials");
    outf        = ajAcdGetOutfile("outfile");


    sscanf(ajStrStr(*menu),"%d",&n);
    --n;
    
    substr=ajStrNew();
    be=ajSeqBegin(a);
    en=ajSeqEnd(a);
    ajStrAssSubC(&substr,ajSeqChar(a),be-1,en-1);
    len = en-be+1;

    l  = ajListNew();
    pa = ajListNew();
    rname = ajStrNew();
    
    
    embPropCalcFragments(ajStrStr(substr),n,be,&l,&pa,unfavoured,overlap,
			allpartials,&ncomp,&npart,&rname);
    

    ajFmtPrintF(outf,"DIGEST of %s from %d to %d Molwt=%10.3f\n\n",
		ajSeqName(a),be,en,embPropCalcMolwt(ajSeqChar(a),0,len-1));
    if(!ncomp)
	ajFmtPrintF(outf,"Is not proteolytically digested using %s\n",
		    ajStrStr(rname));
    else
    {
	ajFmtPrintF(outf,"Complete digestion with %s yields %d fragments:\n",
		    ajStrStr(rname),ncomp);
	digest_print_hits(l,outf,be,ajStrStr(substr));
    }
  
    if(overlap && !allpartials && npart)
    {
	ajFmtPrintF(outf,"\n\nPartial digest with %s yields %d extras.\n",
		    ajStrStr(rname),npart);
	ajFmtPrintF(outf,"Only overlapping partials shown:\n");
	digest_print_hits(pa,outf,be,ajStrStr(substr));
    }

    if(allpartials && npart)
    {
	ajFmtPrintF(outf,"\n\nPartial digest with %s yields %d extras.\n",
		    ajStrStr(rname),npart);
	ajFmtPrintF(outf,"All partials shown:\n");
	digest_print_hits(pa,outf,be,ajStrStr(substr));
    }
    

    ajSeqDel(&a);
    ajStrDel(&rname);
    ajStrDel(&substr);
    ajListDel(&pa);
    ajListDel(&l);
    
    ajFileClose(&outf);

    ajExit();
    return 0;
}


/* @funcstatic digest_print_hits **********************************************
**
** Undocumented.
**
** @param [?] l [AjPList] Undocumented
** @param [?] outf [AjPFile] Undocumented
** @param [?] be [ajint] Undocumented
** @param [?] s [char*] Undocumented
** @@
******************************************************************************/



void digest_print_hits(AjPList l, AjPFile outf, ajint be, char *s)
{
    EmbPPropFrag fr;
    AjPStr  t;
    ajint     len;
    
    t=ajStrNew();
    len=strlen(s);
    
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
	AJFREE (fr);
    }

    ajStrDel(&t);
    
    return;
}

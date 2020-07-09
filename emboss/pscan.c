/* @source pscan application
**
** Reports fingerprints in a protein sequence
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



void print_hits(AjPFile *outf, AjPList *l, ajint nmotifs, AjPStr *name,
		ajint begin, ajint end);






int main(int argc, char **argv)
{
    AjPSeqall seqall;
    AjPSeq    seq=NULL;
    AjPFile   outf=NULL;
    ajint begin;
    ajint end;
    ajint emin;
    ajint emax;
    
    EmbPMatMatch mm=NULL;
    
    AjPFile   mfile=NULL;
    EmbPMatPrints s=NULL;
    AjPList   l=NULL;
    AjPStr    strand=NULL;
    AjPStr    substr=NULL;
    AjPStr    name=NULL;
    
    AjBool    all;
    AjBool    ordered;
    
    ajint hits;
    ajint nmotifs;
    

    embInit("pscan", argc, argv);

    seqall    = ajAcdGetSeqall("sequence");
    outf      = ajAcdGetOutfile("outfile");
    emin      = ajAcdGetInt("emin");
    emax      = ajAcdGetInt("emax");
    
    substr=ajStrNew();
    name =ajStrNew();
    
    all=ordered=ajTrue;


    
    while(ajSeqallNext(seqall, &seq))
    {
/*	ajUser("Scanning %s...",ajSeqName(seq)); */
	begin=ajSeqallBegin(seqall);
	end=ajSeqallEnd(seqall);
	
	ajStrAssC(&name,ajSeqName(seq));
	strand=ajSeqStrCopy(seq);

	ajStrAssSubC(&substr,ajStrStr(strand),begin-1,end-1);

	nmotifs=0;
	embMatPrintsInit(&mfile);
	l=ajListNew();
	while(embMatProtReadInt(&mfile,&s))
	{
	    if(s->n >= emin && s->n <= emax)
	    {
		hits=embMatProtScanInt(&substr,&name,&s,&l,&all,&ordered,
					    1);
		if(hits)
		    ++nmotifs;
	    }
	    embMatProtDelInt(&s);
	}

	ajListPop(l,(void **)&mm);

/*	ajUser(""); */
	print_hits(&outf, &l, nmotifs, &name, begin, end);
	ajListDel(&l);


	ajStrDel(&strand);    
	ajFileClose(&mfile);
    }



    

    ajStrDel(&substr);
    ajSeqDel(&seq);
    ajFileClose(&outf);
    ajExit();
    return 0;
}









void print_hits(AjPFile *outf, AjPList *l, ajint nmotifs, AjPStr *name,
		ajint begin, ajint end)
{
    EmbPMatMatch mm;
    ajint i;
    ajint j;
    AjBool found;
    ajint nleft;
    ajint maxelem=0;
    ajint maxhpm;
    ajint hpm=0;

    nleft = nmotifs;
    

    ajFmtPrintF(*outf,"\n\nCLASS 1\n");
    ajFmtPrintF(*outf,"Fingerprints with all elements in order\n\n");


    found = ajTrue;

    while(found && nleft)
    {
	found = ajFalse;
	maxelem = 0;
	for(i=0;i<nleft;++i)
	{
	    ajListPop(*l,(void **)&mm);
	    if(mm->all && mm->ordered)
	    {
		maxelem = AJMAX(maxelem,mm->n);
		found = ajTrue;
	    }
	    hpm = mm->hpm;
	    
	    ajListPushApp(*l,(void *)mm);
	    for(j=1;j<hpm;++j)
	    {
		ajListPop(*l,(void **)&mm);
		ajListPushApp(*l,(void *)mm);
	    }
	}


	if(found)
	{

	    found=ajFalse;
	    for(i=0;i<nleft;++i)
	    {
		ajListPop(*l,(void **)&mm);
		hpm = mm->hpm;
		if(mm->all && mm->ordered && maxelem==mm->n)
		    break;
		ajListPushApp(*l,(void *)mm);
		for(j=1;j<hpm;++j)
		{
		    ajListPop(*l,(void **)&mm);
		    ajListPushApp(*l,(void *)mm);
		}
	    }

	    if(mm->all && mm->ordered && maxelem==mm->n)
	    {
	    ajFmtPrintF(*outf,"Fingerprint %s Elements %d\n",
			ajStrStr(mm->cod),mm->n);
	    ajFmtPrintF(*outf,"    Accession number %s\n",ajStrStr(mm->acc));
	    ajFmtPrintF(*outf,"    %s\n",ajStrStr(mm->tit));
	    ajFmtPrintF(*outf,"  Element %d Threshold %d%% Score %d%%\n",
			mm->element+1, mm->thresh, mm->score);
	    ajFmtPrintF(*outf,"             Start position %d Length %d\n",
			mm->start+begin,mm->len);

	    embMatMatchDel(&mm);
	    }
	    
	    for(i=1;i<hpm;++i)
	    {
		ajListPop(*l,(void **)&mm);
		ajFmtPrintF(*outf,"  Element %d Threshold %d%% Score %d%%\n",
			    mm->element+1, mm->thresh, mm->score);
		ajFmtPrintF(*outf,"             Start position %d Length %d\n",
			    mm->start+begin,mm->len);
	    
		embMatMatchDel(&mm);
	    }
	    
	    --nleft;
	}
	
    }
    




    ajFmtPrintF(*outf,"\n\nCLASS 2\n");	
    ajFmtPrintF(*outf,"All elements match but not all in the correct order\n\n");	

    found = ajTrue;

    while(found && nleft)
    {
	found = ajFalse;
	maxelem = 0;
	for(i=0;i<nleft;++i)
	{
	    ajListPop(*l,(void **)&mm);
	    if(mm->all)
	    {
		maxelem = AJMAX(maxelem,mm->n);
		found = ajTrue;
	    }
	    hpm = mm->hpm;
	    ajListPushApp(*l,(void *)mm);
	    for(j=1;j<hpm;++j)
	    {
		ajListPop(*l,(void **)&mm);
		ajListPushApp(*l,(void *)mm);
	    }
	}


	if(found)
	{

	    found=ajFalse;
	    for(i=0;i<nleft;++i)
	    {
		ajListPop(*l,(void **)&mm);
		hpm = mm->hpm;
		if(mm->all && maxelem==mm->n)
		    break;
		ajListPushApp(*l,(void *)mm);
		for(j=1;j<hpm;++j)
		{
		    ajListPop(*l,(void **)&mm);
		    ajListPushApp(*l,(void *)mm);
		}
	    }
	    if(mm->all && maxelem==mm->n)
	    {
	    ajFmtPrintF(*outf,"Fingerprint %s Elements %d\n",
			ajStrStr(mm->cod),mm->n);
	    ajFmtPrintF(*outf,"    Accession number %s\n",ajStrStr(mm->acc));
	    ajFmtPrintF(*outf,"    %s\n",ajStrStr(mm->tit));
	    ajFmtPrintF(*outf,"  Element %d Threshold %d%% Score %d%%\n",
			mm->element+1, mm->thresh, mm->score);
	    ajFmtPrintF(*outf,"             Start position %d Length %d\n",
			mm->start+begin,mm->len);

	    embMatMatchDel(&mm);
	    }
	    
	    for(i=1;i<hpm;++i)
	    {
		ajListPop(*l,(void **)&mm);
		ajFmtPrintF(*outf,"  Element %d Threshold %d%% Score %d%%\n",
			    mm->element+1, mm->thresh, mm->score);
		ajFmtPrintF(*outf,"             Start position %d Length %d\n",
			    mm->start+begin,mm->len);
	    
		embMatMatchDel(&mm);
	    }
	    
	    --nleft;
	}
	
    }




    ajFmtPrintF(*outf,"\n\nCLASS 3\n");
    ajFmtPrintF(*outf,
		"Not all elements match but those that do are in order\n\n");

    found = ajTrue;

    while(found && nleft)
    {
	found = ajFalse;
	maxhpm = 0;
	for(i=0;i<nleft;++i)
	{
	    ajListPop(*l,(void **)&mm);
	    if(mm->ordered)
	    {
		maxhpm = AJMAX(maxelem,mm->hpm);
		found = ajTrue;
	    }
	    hpm = mm->hpm;
	    ajListPushApp(*l,(void *)mm);
	    for(j=1;j<hpm;++j)
	    {
		ajListPop(*l,(void **)&mm);
		ajListPushApp(*l,(void *)mm);
	    }
	}


	if(found)
	{

	    found=ajFalse;
	    for(i=0;i<nleft;++i)
	    {
		ajListPop(*l,(void **)&mm);
		hpm = mm->hpm;
		if(mm->ordered && maxhpm==mm->hpm)
		    break;
		ajListPushApp(*l,(void *)mm);
		for(j=1;j<hpm;++j)
		{
		    ajListPop(*l,(void **)&mm);
		    ajListPushApp(*l,(void *)mm);
		}
	    }
	    if(mm->ordered && maxhpm==mm->hpm)
	    {
	    ajFmtPrintF(*outf,"Fingerprint %s Elements %d\n",
			ajStrStr(mm->cod),mm->n);
	    ajFmtPrintF(*outf,"    Accession number %s\n",ajStrStr(mm->acc));
	    ajFmtPrintF(*outf,"    %s\n",ajStrStr(mm->tit));
	    ajFmtPrintF(*outf,"  Element %d Threshold %d%% Score %d%%\n",
			mm->element+1, mm->thresh, mm->score);
	    ajFmtPrintF(*outf,"             Start position %d Length %d\n",
			mm->start+begin,mm->len);
	    

            embMatMatchDel(&mm);
	    }
	    
	    for(i=1;i<hpm;++i)
	    {
		ajListPop(*l,(void **)&mm);
		ajFmtPrintF(*outf,"  Element %d Threshold %d%% Score %d%%\n",
			    mm->element+1, mm->thresh, mm->score);
		ajFmtPrintF(*outf,"             Start position %d Length %d\n",
			    mm->start+begin,mm->len);
	    
		embMatMatchDel(&mm);
	    }
	    
	    --nleft;
	}
	
    }




    ajFmtPrintF(*outf,"\n\nCLASS 4\n");
    ajFmtPrintF(*outf,
		"Remaining partial matches\n\n");

    found = ajTrue;

    while(found && nleft)
    {
	found = ajFalse;
	maxhpm = 0;
	for(i=0;i<nleft;++i)
	{
	    ajListPop(*l,(void **)&mm);
	    maxhpm = AJMAX(maxelem,mm->hpm);
	    found = ajTrue;

	    hpm = mm->hpm;
	    ajListPushApp(*l,(void *)mm);
	    for(j=1;j<hpm;++j)
	    {
		ajListPop(*l,(void **)&mm);
		ajListPushApp(*l,(void *)mm);
	    }
	}


	if(found)
	{

	    found=ajFalse;
	    for(i=0;i<nleft;++i)
	    {
		ajListPop(*l,(void **)&mm);
		hpm = mm->hpm;
		if(maxhpm==mm->hpm)
		    break;
		ajListPushApp(*l,(void *)mm);
		for(j=1;j<hpm;++j)
		{
		    ajListPop(*l,(void **)&mm);
		    ajListPushApp(*l,(void *)mm);
		}
	    }
	    if(maxhpm==mm->hpm)
	    {
	    ajFmtPrintF(*outf,"Fingerprint %s Elements %d\n",
			ajStrStr(mm->cod),mm->n);
	    ajFmtPrintF(*outf,"    Accession number %s\n",ajStrStr(mm->acc));
	    ajFmtPrintF(*outf,"    %s\n",ajStrStr(mm->tit));
	    ajFmtPrintF(*outf,"  Element %d Threshold %d%% Score %d%%\n",
			mm->element+1, mm->thresh, mm->score);
	    ajFmtPrintF(*outf,"             Start position %d Length %d\n",
			mm->start+begin,mm->len);
	    

	    embMatMatchDel(&mm);
	    }
	    
	    for(i=1;i<hpm;++i)
	    {
		ajListPop(*l,(void **)&mm);
		ajFmtPrintF(*outf,"  Element %d Threshold %d%% Score %d%%\n",
			    mm->element+1, mm->thresh, mm->score);
		ajFmtPrintF(*outf,"             Start position %d Length %d\n",
			    mm->start+begin,mm->len);
	    
		embMatMatchDel(&mm);
	    }
	    
	    --nleft;
	}
	
    }
    

    while(ajListPop(*l,(void **)&mm))
	if(mm)
	    embMatMatchDel(&mm);
    
    return;
}

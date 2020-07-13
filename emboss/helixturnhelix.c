/* @source helixturnhelix application
**
** Reports nucleic acid binding domains
** @author: Copyright (C) Alan Bleasby (ableasby@hgmp.mrc.ac.uk)
** @@
**
** Original program "HELIXTURNHELIX" by Peter Rice (EGCG 1990)
** This program uses the method of Dodd and Egan (1987) J. Mol. Biol.
** 194:557-564 to determine the significance of possible helix-turn-helix
** matches in protein sequences
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
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define HTHFILE "Ehth.dat"
#define HTH87FILE "Ehth87.dat"

typedef struct DNAB DNAB;
struct DNAB
{
    ajint pos;
    AjPStr name;
    AjPStr seq;
    float  sd;
    ajint    wt;
}
;



static ajint hth_readNab(AjPInt2d *matrix,AjBool eightyseven);
static void hth_print_hits(AjPList *ajb, ajint n, float minsd, ajint lastcol,
			   AjBool eightyseven, AjPFile outf);
static void hth_report_hits(AjPList *ajb, ajint lastcol,
			    AjPReport report,
			    AjPFeattable TabRpt);



/* @prog helixturnhelix *******************************************************
**
** Report nucleic acid binding motifs
**
******************************************************************************/

int main(int argc, char **argv)
{
    AjPSeqall seqall;
    AjPSeq    seq=NULL;
    AjPFile   outf=NULL;
    AjPReport report=NULL;
    AjPList   ajb=NULL;
    AjPStr    strand=NULL;
    AjPStr    substr=NULL;
    AjBool    eightyseven=ajFalse;
    float     mean;
    float     sd;
    float     minsd;
    static DNAB      *lp;

    AjPInt2d matrix=NULL;
    AjPStr    tmpStr = NULL;
    AjPFeattable TabRpt = NULL;

    ajint begin;
    ajint end;
    ajint len;

    char *p;
    char *q;

    ajint i;
    ajint j;
    ajint cols;
    ajint lastcol;

    ajint n=0;

    ajint sp;
    ajint se;
    ajint weight;

    float minscore;
    float thissd;

    embInit("helixturnhelix",argc,argv);

    seqall    = ajAcdGetSeqall("sequence");
    report    = ajAcdGetReport("outfile");
    mean      = ajAcdGetFloat("mean");
    sd        = ajAcdGetFloat("sd");
    minsd     = ajAcdGetFloat("minsd");

    /* obsolete. Can be uncommented in acd file and here to reuse */

    /* outf      = ajAcdGetOutfile("originalfile"); */

    substr = ajStrNew();
    matrix = ajInt2dNew();

    eightyseven = ajAcdGetBool("eightyseven");

    cols=hth_readNab(&matrix,eightyseven);
    ajDebug("cols = %d\n",cols);

    lastcol = cols-3;

    minscore = mean + (minsd*sd);

    ajb=ajListNew();

    (void) ajFmtPrintAppS(&tmpStr,
			  "Hits above +%.2f SD (%.2f)",
			  minsd,minscore);
    ajReportSetHeader(report, tmpStr);

    while(ajSeqallNext(seqall, &seq))
    {
	n=0;
	begin = ajSeqallBegin(seqall);
	end   = ajSeqallEnd(seqall);


	strand = ajSeqStrCopy(seq);
	ajStrToUpper(&strand);

	ajStrAssSubC(&substr,ajStrStr(strand),begin-1,end-1);
	len    = ajStrLen(substr);

	TabRpt = ajFeattableNewSeq(seq);

	q = p = ajStrStr(substr);
	for(i=0;i<len;++i,++p)
	    *p = (char) ajAZToInt(*p);
	p=q;

	se = (len-lastcol)+1;
	for(i=0;i<se;++i)
	{
	    weight=0;
	    for(j=0;j<lastcol;++j)
		weight+=ajInt2dGet(matrix,(ajint)*(p+i+j),j);
	    thissd=((float)weight-mean)/sd;
	    if(thissd>minsd)
	    {
		AJNEW(lp);
		lp->name=ajStrNewC(ajSeqName(seq));
		lp->seq =ajStrNew();
		sp = begin - 1 + i;
		lp->pos = sp+1;
		ajStrAssSubC(&lp->seq,ajStrStr(strand),sp,sp+lastcol-1);
		lp->sd = thissd;
		lp->wt = weight;
		ajListPush(ajb,(void *)lp);
		++n;
	    }
	}
	hth_report_hits(&ajb, lastcol, report, TabRpt);

	ajReportWrite(report, TabRpt, seq);
	ajFeattableDel(&TabRpt);
	ajStrDel(&strand);
    }


    if(!n) {
	if (outf) {
	  ajFmtPrintF(outf,"\nNo hits above +%.2f SD (%.2f)\n",
		      minsd,minscore);
	}
    }
    else
    {
        if (outf) {
	  ajFmtPrintF(outf,
		      "\nHELIXTURNHELIX: Nucleic Acid Binding Domain search\n\n");
	  ajFmtPrintF(outf,"\nHits above +%.2f SD (%.2f)\n",minsd,minscore);
	  hth_print_hits(&ajb, n, minsd, lastcol, eightyseven, outf);
	}
    }

    ajInt2dDel(&matrix);

    ajSeqDel(&seq);
    ajStrDel(&substr);
    ajListDel(&ajb);
    if (outf)
      ajFileClose(&outf);

    (void) ajReportClose(report);

    ajExit();
    return 0;
}




/* @funcstatic hth_readNab ****************************************************
**
** Undocumented.
**
** @param [?] matrix [AjPInt2d*] Undocumented
** @param [?] eightyseven [AjBool] Undocumented
** @return [ajint] Undocumented
** @@
******************************************************************************/


static ajint hth_readNab(AjPInt2d *matrix,AjBool eightyseven)
{
    AjPFile mfptr=NULL;
    AjPStr  line=NULL;
    AjPStr  delim=NULL;
    AjBool  pass;

    char *p;
    char *q;

    ajint xcols=0;
    ajint cols=0;

    float sample;
    float expected;
    float pee;
    float exptot;
    ajint   rt;

    ajint   i;
    ajint   j;
    ajint   c=0;
    ajint   v;

    ajint d1;
    ajint d2;

    ajint **mat;

    if(eightyseven)
	ajFileDataNewC(HTH87FILE,&mfptr);
    else
	ajFileDataNewC(HTHFILE,&mfptr);
    if(!mfptr) ajFatal("HTH file not found\n");

    line=ajStrNew();
    delim=ajStrNewC(" :\t\n");

    pass = ajTrue;

    while(ajFileGets(mfptr, &line))
    {
	p=ajStrStr(line);
	if(*p=='#' || *p=='!' || *p=='\n') continue;
	if(ajStrPrefixC(line,"Sample:"))
	{
	    if(sscanf(p,"%*s%f",&sample)!=1)
		ajFatal("No sample size given");
	    continue;
	}
	while((*p!='\n') && (*p<'A' || *p>'Z')) ++p;
	cols = ajStrTokenCount(&line,ajStrStr(delim));
	if(pass)
	{
	    pass=ajFalse;
	    xcols = cols;
	}
	else
	    if(xcols!=cols)
		ajFatal("Assymetric table");

	d1 = ajAZToInt((char)toupper((ajint)*p));

	q=ajStrStr(line);
	c = 0;
	q = ajSysStrtok(q,ajStrStr(delim));
	while((q=ajSysStrtok(NULL,ajStrStr(delim))))
	{
	    (void) sscanf(q,"%d",&v);
	    ajInt2dPut(matrix,d1,c++,v);
	}

	for(i=0,rt=0;i<c-2;++i) rt+=ajInt2dGet(*matrix,d1,i);

	if(rt!=ajInt2dGet(*matrix,d1,c-2))
	    ajFatal("Row didn't match total");
    }


    mat = ajInt2dInt(*matrix);
    ajInt2dLen(*matrix,&d1,&d2);


    for(j=0;j<d2-2;++j)
    {
	rt=0;
	for(i=0;i<d1;++i)
	{
	    if(!mat[i][d2-1]) continue;
	    rt += mat[i][j];
	}
	if(rt!=(ajint)sample)
	    ajFatal("Column doesn't match sample size");
    }

    exptot=0.0;
    for(i=0;i<d1;++i)
    {
	if(!mat[i][d2-1]) continue;
	expected = mat[i][c-1];
	expected *= 0.0001;
	exptot += expected;
	for(j=0;j<c-2;++j)
	{
	    if(!mat[i][j]) pee=(1.0<1.0/((sample+1.0)*expected)) ? 1.0 :
		1.0/((sample+1.0)*expected);
	    else
		pee = ((float)mat[i][j])/(sample*expected);
	    mat[i][j]=(ajint)((double)100.0*log(pee));
	}
    }
    if((float)fabs((double)(1.0-exptot)) > 0.05)
	ajFatal("Expected column total != 1.0");

    for(i=0;i<d1;++i)
	for(j=0;j<d2;++j)
	    ajInt2dPut(matrix,i,j,mat[i][j]);

    for(i=0;i<d1;++i)
	AJFREE(mat[i]);
    AJFREE(mat);

    ajStrDel(&line);
    ajStrDel(&delim);
    ajFileClose(&mfptr);

    return cols;
}





/* @funcstatic hth_print_hits *************************************************
**
** Undocumented.
**
** @param [?] ajb [AjPList*] Undocumented
** @param [?] n [ajint] Undocumented
** @param [?] minsd [float] Undocumented
** @param [?] lastcol [ajint] Undocumented
** @param [?] eightyseven [AjBool] Undocumented
** @param [?] outf [AjPFile] Undocumented
** @@
******************************************************************************/


static void hth_print_hits(AjPList *ajb, ajint n, float minsd, ajint lastcol,
			   AjBool eightyseven, AjPFile outf)
{
    DNAB     **lp;

    AjPInt   hp=NULL;
    AjPFloat hsd=NULL;

    ajint   i;

    AJCNEW (lp, n);

    hp  = ajIntNew();
    hsd = ajFloatNew();

    for(i=0;i<n;++i)
    {
	if(!ajListPop(*ajb,(void **)&lp[i]))
	    ajFatal("Poppa doesn't live here anymore");
	ajIntPut(&hp,i,i);
	ajFloatPut(&hsd,i,lp[i]->sd);
    }
    ajSortFloatIncI(ajFloatFloat(hsd),ajIntInt(hp),n);
    ajFloatDel(&hsd);

    for(i=0;i<n;++i)
    {
	ajFmtPrintF(outf,"\nScore %d (+%.2f SD) in %s at residue %d\n",
		   lp[ajIntGet(hp,i)]->wt,lp[ajIntGet(hp,i)]->sd,
		    ajStrStr(lp[ajIntGet(hp,i)]->name),
		   lp[ajIntGet(hp,i)]->pos);
	ajFmtPrintF(outf,"\n Sequence:  %s\n",
		    ajStrStr(lp[ajIntGet(hp,i)]->seq));
	if(eightyseven)
	{
	    ajFmtPrintF(outf,"            |                  |\n");
	    ajFmtPrintF(outf,"%13d                  %d\n",
			lp[ajIntGet(hp,i)]->pos,
			lp[ajIntGet(hp,i)]->pos+lastcol-1);
	}
	else
	{
	    ajFmtPrintF(outf,"            |                    |\n");
	    ajFmtPrintF(outf,"%13d                    %d\n",
			lp[ajIntGet(hp,i)]->pos,
			lp[ajIntGet(hp,i)]->pos+lastcol-1);
	}
    }

    /*
     *  Tidy up memory
     */
    for(i=0;i<n;++i)
    {
	ajStrDel(&lp[i]->name);
	ajStrDel(&lp[i]->seq);
    }
    AJFREE (lp);
    ajIntDel(&hp);

    return;
}

/* @funcstatic hth_report_hits ************************************************
**
** Undocumented.
**
** @param [?] ajb [AjPList*] Undocumented
** @param [?] lastcol [ajint] Undocumented
** @param [?] report [AjPReport] Undocumented
** @param [?] TabRpt [AjPFeattable] Undocumented
** @return [void]
** @@
******************************************************************************/


static void hth_report_hits(AjPList *ajb, ajint lastcol,
			    AjPReport report,
			    AjPFeattable TabRpt)
{
    DNAB     **lp;

    AjPInt   hp=NULL;
    AjPFloat hsd=NULL;

    ajint n;
    ajint i;
    AjPFeature gf = NULL;

    AjPStr tmpStr = NULL;
    static AjPStr fthit = NULL;
    struct DNAB *dnab;

    if (!fthit)
      ajStrAssC(&fthit, "hit");

    /* AJCNEW (lp, n);*/

    hp  = ajIntNew();
    hsd = ajFloatNew();

    n = ajListToArray(*ajb, (void***) &lp);

    if (!n) return;

    for(i=0;i<n;++i)
    {
      /*
	if(!ajListPop(*ajb,(void **)&lp[i]))
	    ajFatal("Fatal bug. List of hits ended too soon");
      */
	ajIntPut(&hp,i,i);
	ajFloatPut(&hsd,i,lp[i]->sd);
    }
    ajSortFloatIncI(ajFloatFloat(hsd),ajIntInt(hp),n);
    ajFloatDel(&hsd);

    for(i=0;i<n;++i)
    {
        gf = ajFeatNewProt (TabRpt, NULL, fthit,
			    lp[ajIntGet(hp,i)]->pos,
			    lp[ajIntGet(hp,i)]->pos+lastcol-1,
			    lp[ajIntGet(hp,i)]->wt);
	ajFmtPrintS(&tmpStr, "*pos %.2f", lp[ajIntGet(hp,i)]->pos);
	ajFeatTagAdd (gf, NULL, tmpStr);
	ajFmtPrintS(&tmpStr, "*sd %.2f", lp[ajIntGet(hp,i)]->sd);
	ajFeatTagAdd (gf, NULL, tmpStr);

    }

    /*
     *  Tidy up memory
     */
    while(ajListPop(*ajb,(void **)&dnab))
    {
	ajStrDel(&dnab->name);
	ajStrDel(&dnab->seq);
	AJFREE(dnab);
    }


/*    for(i=0;i<n;++i)
    {
	ajStrDel(&lp[i]->name);
	ajStrDel(&lp[i]->seq);
    }
*/
    AJFREE (lp);
    ajIntDel(&hp);

    return;
}

/* @source mwfilter application
**
** Remove background keratin, trypsin and sodium noise from
** a file of molecular weights from a mass spec
** Also remove oxidised methionines and threonines
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
*******************************************************************/

#include "emboss.h"

#define DELETED -1.
#define MILLION 1000000.

/*
 *  prototypes
 */
static void mwfilter_readdata(AjPStr datafile, AjPDouble *rmarray,
			      AjPDouble *darray);
static void mwfilter_readexp(AjPFile inf, AjPDouble *exparray);
static void mwfilter_noisedel(AjPDouble exparray, ajint expn,
			      AjPDouble rmarray, ajint rmn,
			      ajint tol);
static void mwfilter_moddel(AjPDouble exparray, ajint expn,
			    AjPDouble darray, ajint dn,
			    ajint tol);
static void mwfilter_arraytidy(AjPDouble exparray, ajint *expn);



/* @prog mwfilter **************************************************
**
** Remove 'noisy' molecular weights
**
*******************************************************************/

int main(int argc, char **argv)
{
    AjPFile inf  = NULL;
    AjPFile outf = NULL;

    float tolerance = 0.0;
    
    AjPStr datafile = NULL;

    AjPDouble rmarray  = NULL;
    AjPDouble darray   = NULL;
    AjPDouble exparray = NULL;
    
    ajint     rmn  = 0;
    ajint     dn   = 0;
    ajint     expn = 0;

    ajint i;
    
    embInit("mwfilter", argc, argv);

    inf       = ajAcdGetInfile("infile");
    tolerance = ajAcdGetFloat("tolerance");
    datafile  = ajAcdGetString("datafile");
    outf      = ajAcdGetOutfile("outfile");

    exparray = ajDoubleNew();
    rmarray  = ajDoubleNew();
    darray   = ajDoubleNew();

    /* Get keratin/trypsin & oxymet/oxythr/sodium data */
    mwfilter_readdata(datafile, &rmarray, &darray);
    rmn = ajDoubleLen(rmarray);
    dn  = ajDoubleLen(darray);

    /* Get experimental results */
    mwfilter_readexp(inf, &exparray);
    expn = ajDoubleLen(exparray);
    
    /* Delete keratin noise etc */
    mwfilter_noisedel(exparray,expn,rmarray,rmn,tolerance);
    mwfilter_arraytidy(exparray,&expn);

    /* Delete oxymet & sodium noise etc */
    mwfilter_moddel(exparray,expn,darray,dn,tolerance);
    mwfilter_arraytidy(exparray,&expn);

    for(i=0;i<expn;++i)
	ajFmtPrintF(outf,"%lf\n",ajDoubleGet(exparray,i));
    
    ajDoubleDel(&exparray);
    ajDoubleDel(&rmarray);
    ajDoubleDel(&darray);
    
    ajFileClose(&inf);
    ajFileClose(&outf);
    
    ajExit();
    return 0;
}




/* @funcstatic  mwfilter_readdata ***********************************
**
** Read molecular weight exclusion file.
**
** @param [r] datafile [AjPStr] Name of datafile (Emwfilter.dat)
** @param [w] rmarray [AjPDouble*] keratin/trypsin data etc
** @param [w] darray [AjPDouble*] oxymet/sodium data etc
** @@
*******************************************************************/
static void mwfilter_readdata(AjPStr datafile, AjPDouble *rmarray,
			      AjPDouble *darray)
{
    AjPFile inf=NULL;
    ajint   rmn = 0;
    ajint   dn  = 0;

    AjPStr  line = NULL;
    char    c;
    double  n;
    
    ajFileDataNew(datafile,&inf);
    if(!inf)
	ajFatal("Cannot open filter data file [%S]",datafile);

    line = ajStrNew();

    /* Read in the top of the file (noisy molwts) */
    while(ajFileReadLine(inf,&line) && !ajStrPrefixC(line,
	  "Displacements"))
    {
	if(!ajStrLen(line))
	    continue;
	c = *ajStrStr(line);
	if(c=='#' || c=='\n')
	    continue;
	if(ajFmtScanS(line,"%*s%lf",&n) != 1)
	    continue;

	ajDoublePut(rmarray,rmn++,n);
    }

    /* If no displacements then return, otherwise read them in */
    /* This copes with oxy-Met, oxy-Thr & Na                   */
    if(!ajStrPrefixC(line,"Displacements"))
    {
	ajStrDel(&line);
	ajFileClose(&inf);
	return;
    }

    while(ajFileReadLine(inf,&line))
    {
	if(!ajStrLen(line))
	    continue;
	c = *ajStrStr(line);
	if(c=='#' || c=='\n')
	    continue;
	if(ajFmtScanS(line,"%*s%lf",&n) != 1)
	    continue;

	ajDoublePut(darray,dn++,n);
    }

    ajStrDel(&line);
    ajFileClose(&inf);
    
    return;
}




/* @funcstatic  mwfilter_readexp ***********************************
**
** Read experimental data.
**
** @param [r] inf [AjPFile] Experimental data
** @param [w] exparray [AjPDouble*] Data array
** @@
*******************************************************************/
static void mwfilter_readexp(AjPFile inf, AjPDouble *exparray)
{
    ajint   expn  = 0;

    AjPStr  line = NULL;
    char    c;
    double  n;
    
    line = ajStrNew();

    while(ajFileReadLine(inf,&line))
    {
	if(!ajStrLen(line))
	    continue;
	c = *ajStrStr(line);
	if(c=='#' || c=='\n')
	    continue;
	if(ajFmtScanS(line,"%lf",&n) != 1)
	    continue;

	ajDoublePut(exparray,expn++,n);
    }

    ajStrDel(&line);
    
    return;
}




/* @funcstatic  mwfilter_noisedel **********************************
**
** Mark as DELETED keratin/trypsin peaks.
**
** @param [rw] exparray [AjPDouble] Experimental data
** @param [r] expn [ajint] Number of experimental peaks
** @param [r] rmarray [AjPDouble] keratin/trypsin data etc
** @param [r] rmn [ajint] Number of keratin/trypsin peaks
** @param [r] tol [ajint] Tolerance (ppm)
** @@
*******************************************************************/
static void mwfilter_noisedel(AjPDouble exparray, ajint expn,
			      AjPDouble rmarray, ajint rmn,
			      ajint tol)
{
    ajint i;
    ajint j;
    double mwmin;
    double mwmax;
    double n;
    double ppmval;
    double mwexp;
    
    for(i=0;i<expn;++i)
    {
	mwexp = ajDoubleGet(exparray,i);
	ppmval = mwexp * tol / MILLION;
	mwmin  = mwexp - ppmval;
	mwmax  = mwexp + ppmval;
	
	for(j=0;j<rmn;++j)
	{
	    n = ajDoubleGet(rmarray,j);
	    if(n>mwmin && n<mwmax)
		ajDoublePut(&exparray,i,DELETED);
	}
    }

    return;
}




/* @funcstatic  mwfilter_arraytidy *********************************
**
** Delete from an array peaks marked as DELETED.
**
** @param [rw] exparray [AjPDouble] Experimental data
** @param [r] expn [ajint*] Number of experimental peaks
** @@
*******************************************************************/
static void mwfilter_arraytidy(AjPDouble exparray, ajint *expn)
{
    ajint i;
    ajint j;
    ajint n;
    ajint limit = *expn;

    double v=0.;
    
    for(i=0,n=0;i<limit;++i)
	if(ajDoubleGet(exparray,i) != DELETED)
	{
	    ++n;
	    continue;
	}
	else
	{
	    for(j=i;j<limit;++j)
		if(j!=limit-1)
		{
		    v = ajDoubleGet(exparray,j+1);
		    ajDoublePut(&exparray,j,v);
		}
	    --limit;
	}

    *expn = n;
    return;
}




/* @funcstatic  mwfilter_moddel ************************************
**
** Mark as DELETED oxymet/sodium peaks.
**
** @param [rw] exparray [AjPDouble] Experimental data
** @param [r] expn [ajint] Number of experimental peaks
** @param [r] darray [AjPDouble] oxymet/sodium molwts etc
** @param [r] dn [ajint] Number of oxymet/sodium molwts
** @param [r] tol [ajint] Tolerance (ppm)
** @@
*******************************************************************/
static void mwfilter_moddel(AjPDouble exparray, ajint expn,
			    AjPDouble darray, ajint dn,
			    ajint tol)
{
    ajint i;
    ajint j;
    ajint k;
    double mwmin;
    double mwmax;
    double n;
    double ppmval;
    double mwexp;
    double mwmod;
    double modmin;
    double modmax;
    
    for(k=0;k<dn;++k)
    {
	mwmod  = ajDoubleGet(darray,k);
	ppmval = mwmod *tol / MILLION;
	modmin = mwmod - ppmval;
	modmax = mwmod + ppmval;

	for(i=0;i<expn;++i)
	{
	    mwexp = ajDoubleGet(exparray,i);
	    if(mwexp == DELETED)
		continue;
	    mwmin = mwexp + modmin;
	    mwmax = mwexp + modmax;
	    
	    for(j=i+1;j<expn;++j)
	    {
		n = ajDoubleGet(exparray,j);
		if(n == DELETED)
		    continue;
		if(n>mwmin && n<mwmax)
		    ajDoublePut(&exparray,i,DELETED);
	    }
	    
	}
    }

    return;
}

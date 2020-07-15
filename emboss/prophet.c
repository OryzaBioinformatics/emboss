/* @source prophet application
**
** Gapped alignment for profiles
** @author Copyright (C) Alan Bleasby (ableasby@hgmp.mrc.ac.uk)
** @modified July 5 2006 Jon Ison (to use align ACD definition)
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
#include <string.h>

#define AZ 28




static ajint prophet_getType(AjPFile inf, AjPStr *tname);

static void prophet_read_profile(AjPFile inf, AjPStr *name, AjPStr *mname,
				 ajint *mlen, float *gapopen,
				 float *gapextend, ajint *thresh,
				 float *maxs, AjPStr *cons);

static void prophet_scan_profile(const AjPStr substr, const AjPStr pname,
				 const AjPStr name, const AjPStr mname,
				 ajint mlen, float * const *fmatrix,
				 ajint thresh, float maxs, float gapopen,
				 float gapextend, AjPAlign align,
				 const AjPStr cons,
				 float opencoeff, float extendcoeff,
				 float *path, ajint *compass, AjPStr *m,
				 AjPStr *n, ajint slen, ajint begin);
/* JISON AjPAlign align replaces AjPOutfile outf */


/* @prog prophet **************************************************************
**
** Gapped alignment for profiles
**
******************************************************************************/

int main(int argc, char **argv)
{
    AjPSeqall seqall;
    AjPSeq seq   = NULL;
    AjPFile inf  = NULL;

    AjPStr strand = NULL;
    AjPStr substr = NULL;
    AjPStr name   = NULL;
    AjPStr mname  = NULL;
    AjPStr tname  = NULL;
    AjPStr pname  = NULL;
    AjPStr line   = NULL;
    AjPStr cons   = NULL;
    AjPStr m      = NULL;
    AjPStr n      = NULL;

    AjPAlign align= NULL; /* JISON, replaces AjPOutfile outf */
    
    ajint type;
    ajint begin;
    ajint end;
    ajint len;
    ajint i;
    ajint j;

    float **fmatrix=NULL;

    ajint mlen;
    ajint maxs = 0;
    float maxfs;
    ajint thresh;

    float gapopen;
    float gapextend;
    float opencoeff;
    float extendcoeff;

    const char *p;

    ajint maxarr = 1000;
    ajint alen;
    float *path;
    ajint *compass;


    embInit("prophet", argc, argv);

    seqall      = ajAcdGetSeqall("sequence");
    inf         = ajAcdGetInfile("infile");
    opencoeff   = ajAcdGetFloat("gapopen");
    extendcoeff = ajAcdGetFloat("gapextend");
    align       = ajAcdGetAlign("outfile");  /*JISON replacing outfile */

    opencoeff   = ajRoundF(opencoeff, 8);
    extendcoeff = ajRoundF(extendcoeff, 8);

    substr = ajStrNew();
    name   = ajStrNew();
    mname  = ajStrNew();
    tname  = ajStrNew();
    line   = ajStrNew();
    m      = ajStrNewC("");
    n      = ajStrNewC("");

    type = prophet_getType(inf,&tname);
    if(!type)
      ajFatal("Unrecognised profile/matrix file format");

    prophet_read_profile(inf,&pname,&mname,&mlen,&gapopen,&gapextend,&thresh,
			 &maxfs, &cons);
    AJCNEW(fmatrix, mlen);

    for(i=0;i<mlen;++i)
    {
	AJCNEW(fmatrix[i], AZ);
	if(!ajFileReadLine(inf,&line))
	    ajFatal("Missing matrix line");
	p = ajStrGetPtr(line);
	p = ajSysStrtok(p," \t");
	for(j=0;j<AZ;++j)
	{
	    sscanf(p,"%f",&fmatrix[i][j]);
	    p = ajSysStrtok(NULL," \t");
	}
    }

    AJCNEW(path, maxarr);
    AJCNEW(compass, maxarr);

    while(ajSeqallNext(seqall, &seq))
    {
	begin = ajSeqallBegin(seqall);
	end   = ajSeqallEnd(seqall);

	ajStrAssignC(&name,ajSeqName(seq));
	strand = ajSeqStrCopy(seq);

	ajStrAssignSubC(&substr,ajStrGetPtr(strand),begin-1,end-1);
	len = ajStrGetLen(substr);

	alen = len*mlen;
	if(alen>maxarr)
	{
	    AJCRESIZE(path,alen);
	    AJCRESIZE(compass,alen);
	    maxarr=alen;
	}

	ajStrAssignC(&m,"");
	ajStrAssignC(&n,"");

	/* JISON used to be
	prophet_scan_profile(substr,pname,name,mname,mlen,fmatrix,thresh,maxs,
			     gapopen,gapextend,outf,cons,opencoeff,
			     extendcoeff,path,compass,&m,&n,len,begin); */

	/* JISON new call and reset align */
	prophet_scan_profile(substr,name,pname,mname,mlen,fmatrix,thresh,
			     maxs,gapopen,gapextend,align,cons,opencoeff,
			     extendcoeff,path,compass,&m,&n,len,begin); 
	ajAlignReset(align);
	
	ajStrDel(&strand);
    }

    for(i=0;i<mlen;++i)
	AJFREE (fmatrix[i]);
    AJFREE (fmatrix);

    AJFREE(path);
    AJFREE(compass);

    ajStrDel(&line);
    ajStrDel(&cons);
    ajStrDel(&name);
    ajStrDel(&pname);
    ajStrDel(&mname);
    ajStrDel(&tname);
    ajStrDel(&substr);
    ajStrDel(&m);
    ajStrDel(&n);
    ajSeqDel(&seq);
    ajFileClose(&inf);

    ajSeqallDel(&seqall);
    embExit();

    return 0;
}



/* @funcstatic prophet_getType ************************************************
**
** Undocumented.
**
** @param [u] inf [AjPFile] infile
** @param [w] tname [AjPStr*] type name
** @return [ajint] 1=Gribskov 2=Henikoff
** @@
******************************************************************************/

static ajint prophet_getType(AjPFile inf, AjPStr *tname)
{
    AjPStr line = NULL;
    const char *p = NULL;
    ajint ret = 0;

    line = ajStrNew();

    while(ajFileReadLine(inf,&line))
    {
	p = ajStrGetPtr(line);
	if(!*p || *p=='#' || *p=='!' || *p=='\n')
	    continue;
	break;
    }

    if(!strncmp(p,"Gribskov",8))
	ret = 1;

    if(!strncmp(p,"Henikoff",8))
	ret = 2;

    ajStrAssignC(tname,p);

    ajStrDel(&line);

    return ret;
}




/* @funcstatic prophet_read_profile *******************************************
**
** Read Gribskov or Henikoff profile
**
** @param [u] inf [AjPFile] infile
** @param [w] name [AjPStr*] profile name
** @param [w] mname [AjPStr*] matrix name
** @param [w] mlen [ajint*] profile length
** @param [w] gapopen [float*] open penalty
** @param [w] gapextend [float*] extend penalty
** @param [w] thresh [ajint*] score threshold
** @param [w] maxs [float*] maximum score
** @param [w] cons [AjPStr*] consensus sequence
** @@
******************************************************************************/

static void prophet_read_profile(AjPFile inf, AjPStr *name, AjPStr *mname,
				 ajint *mlen, float *gapopen,
				 float *gapextend, ajint *thresh,
				 float *maxs, AjPStr *cons)
{
    const char *p;
    AjPStr line = NULL;

    line = ajStrNew();

    if(!ajFileReadLine(inf,&line))
	ajFatal("Premature EOF in profile file");
    p = ajStrGetPtr(line);

    if(strncmp(p,"Name",4))
	ajFatal("Incorrect profile/matrix file format");

    p = ajSysStrtok(p," \t");
    p = ajSysStrtok(NULL," \t");
    ajStrAssignC(name,p);

    if(!ajFileReadLine(inf,&line))
	ajFatal("Premature EOF in profile file");

    p = ajStrGetPtr(line);
    if(strncmp(p,"Matrix",6))
	ajFatal("Incorrect profile/matrix file format");
    p = ajSysStrtok(p," \t");
    p = ajSysStrtok(NULL," \t");
    ajStrAssignC(mname,p);


    if(!ajFileReadLine(inf,&line))
	ajFatal("Premature EOF in profile file");
    p = ajStrGetPtr(line);

    if(strncmp(p,"Length",6))
	ajFatal("Incorrect profile/matrix file format");
    sscanf(p,"%*s%d",mlen);

    if(!ajFileReadLine(inf,&line))
	ajFatal("Premature EOF in profile file");
    p = ajStrGetPtr(line);

    if(strncmp(p,"Max_score",9))
	ajFatal("Incorrect profile/matrix file format");
    sscanf(p,"%*s%f",maxs);

    if(!ajFileReadLine(inf,&line))
	ajFatal("Premature EOF in profile file");
    p = ajStrGetPtr(line);

    if(strncmp(p,"Threshold",9))
	ajFatal("Incorrect profile/matrix file format");
    sscanf(p,"%*s%d",thresh);


    if(!ajFileReadLine(inf,&line))
	ajFatal("Premature EOF in profile file");
    p = ajStrGetPtr(line);

    if(strncmp(p,"Gap_open",8))
	ajFatal("Incorrect profile/matrix file format");
    sscanf(p,"%*s%f",gapopen);

    if(!ajFileReadLine(inf,&line))
	ajFatal("Premature EOF in profile file");
    p = ajStrGetPtr(line);

    if(strncmp(p,"Gap_extend",10))
	ajFatal("Incorrect profile/matrix file format");
    sscanf(p,"%*s%f",gapextend);

    if(!ajFileReadLine(inf,&line))
	ajFatal("Premature EOF in profile file");
    p = ajStrGetPtr(line);

    if(strncmp(p,"Consensus",9))
	ajFatal("Incorrect profile/matrix file format");
    p = ajSysStrtok(p," \t\n");
    p = ajSysStrtok(NULL," \t\n");
    ajStrAssignC(cons,p);

    ajStrDel(&line);

    return;
}






/* @funcstatic prophet_scan_profile ******************************************
**
** Scan sequence with profile
**
** @param [r] substr [const AjPStr] sequence
** @param [r] name [const AjPStr] seq name
** @param [r] pname [const AjPStr] profilename
** @param [r] mname [const AjPStr] matrix name
** @param [r] mlen [ajint] profile length
** @param [r] fmatrix [float* const *] score matrix
** @param [r] thresh [ajint] score threshold
** @param [r] maxs [float] maximum score
** @param [r] gapopen [float] open penalty
** @param [r] gapextend [float] extend penalty
** @param [u] align [AjPAlign] alignment
** @param [r] cons [const AjPStr] consensus sequence
** @param [r] opencoeff [float] opening co-efficient
** @param [r] extendcoeff [float] extension co-efficient
** @param [w] path [float*] path matrix
** @param [w] compass [ajint*] path direction
** @param [w] m [AjPStr*] sequence result
** @param [w] n [AjPStr*] consensus result
** @param [r] slen [ajint] sequence length
** @param [r] begin [ajint] start position
** @@
******************************************************************************/


static void prophet_scan_profile(const AjPStr substr, const AjPStr name,
				 const AjPStr pname, const AjPStr mname,
				 ajint mlen, float * const *fmatrix,
				 ajint thresh, float maxs, float gapopen,
				 float gapextend, AjPAlign align,
				 const AjPStr cons,
				 float opencoeff, float extendcoeff,
				 float *path, ajint *compass, AjPStr *m,
				 AjPStr *n, ajint slen, ajint begin)
{
    /* JISON AjPAlign align replaces AjPOutfile outf */
    float score;
    ajint start1;
    ajint start2;
    AjPSeq seqm=NULL;
    AjPSeq seqn=NULL;
    

    embAlignProfilePathCalc(ajStrGetPtr(substr),mlen,slen,opencoeff,extendcoeff,
			    path,fmatrix,compass,0);

    score=embAlignScoreProfileMatrix(path,compass,opencoeff,extendcoeff,
				     substr,mlen,slen,fmatrix,&start1,
				     &start2);

    embAlignWalkProfileMatrix(path,compass,opencoeff,extendcoeff,cons,
			      substr,m,n,mlen,slen,fmatrix,&start1,
			      &start2);


    /* JISON ... used to be 
       embAlignPrintProfile(outf,ajStrGetPtr(cons),ajStrGetPtr(substr),*m,*n,
       start1,start2,score,1,fmatrix,"Consensus",
       ajStrGetPtr(pname),1,begin); */

    /* JISON new block */
    seqm=ajSeqNewNameS((const AjPStr)*m, (const AjPStr)pname);
    seqn=ajSeqNewNameS((const AjPStr)*n, (const AjPStr)name); 
    ajAlignDefineSS(align, seqm, seqn);
    ajAlignSetScoreR(align, score);
    ajAlignSetRange(align, 
		    start1, start1+mlen-1, mlen, 1,
		    start2, start2+slen-1, slen, 1);
    ajAlignWrite(align);
    ajSeqDel(&seqm);
    ajSeqDel(&seqn);
    
    return;
}

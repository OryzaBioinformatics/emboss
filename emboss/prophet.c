/* @source prophet application
**
** Gapped alignment for profiles
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
#include <string.h>

#define AZ 28


int  getType(AjPFile inf, AjPStr *tname);

static void read_profile(AjPFile inf, AjPStr *name, AjPStr *mname, int *mlen,
			  float *gapopen, float *gapextend, int *thresh,
			  float *maxs, AjPStr *cons);
void scan_profile(AjPStr substr, AjPStr pname, AjPStr name, AjPStr mname,
		  int mlen, float **fmatrix, int thresh, float maxs,
		  float gapopen, float gapextend, AjPFile outf,
		  AjPStr *cons, float opencoeff, float extendcoeff,
		  float *path, int *compass, AjPStr *m, AjPStr *n,
		  int slen, int begin);



int main( int argc, char **argv, char **env)
{
    AjPSeqall seqall;
    AjPSeq    seq=NULL;
    AjPFile   inf=NULL;
    AjPFile   outf=NULL;
    
    AjPStr    strand=NULL;
    AjPStr    substr=NULL;
    AjPStr    name=NULL;
    AjPStr    mname=NULL;
    AjPStr    tname=NULL;
    AjPStr    pname=NULL;
    AjPStr    line=NULL;
    AjPStr    cons=NULL;
    AjPStr    m=NULL;
    AjPStr    n=NULL;
    
    int       type;
    int       begin;
    int       end;
    int       len;
    int       i;
    int       j;
    
    float **fmatrix=NULL;

    int mlen;
    int maxs=0;
    float maxfs;
    int thresh;

    float gapopen;
    float gapextend;
    float opencoeff;
    float extendcoeff;
    
    char *p;

    int maxarr=1000;
    int alen;
    float *path;
    int   *compass;
    

    embInit("prophet", argc, argv);

    seqall      = ajAcdGetSeqall("sequence");
    inf         = ajAcdGetInfile("infile");
    opencoeff   = ajAcdGetFloat("gapopen");
    extendcoeff = ajAcdGetFloat("gapextend");
    outf        = ajAcdGetOutfile("outfile");
    
    opencoeff = ajRoundF(opencoeff, 8);
    extendcoeff = ajRoundF(extendcoeff, 8);

    seq=ajSeqNew();
    substr=ajStrNew();
    name=ajStrNew();
    mname=ajStrNew();
    tname=ajStrNew();
    line=ajStrNew();
    m=ajStrNewC("");
    n=ajStrNewC("");
    
    type=getType(inf,&tname);
    if(!type)
      ajFatal("Unrecognised profile/matrix file format");
    
    read_profile(inf,&name,&mname,&mlen,&gapopen,&gapextend,&thresh,
		  &maxfs, &cons);
    AJCNEW(fmatrix, mlen);

    for(i=0;i<mlen;++i)
    {
	AJCNEW(fmatrix[i], AZ);
	if(!ajFileReadLine(inf,&line))
	    ajFatal("Missing matrix line");
	p = ajStrStr(line);
	p = strtok(p," \t");
	for(j=0;j<AZ;++j)
	{
	    sscanf(p,"%f",&fmatrix[i][j]);
	    p = strtok(NULL," \t");
	}
    }
    
    AJCNEW(path, maxarr);
    AJCNEW(compass, maxarr);
    
    while(ajSeqallNext(seqall, &seq))
    {
	begin=ajSeqallBegin(seqall);
	end=ajSeqallEnd(seqall);
	
	ajStrAssC(&pname,ajSeqName(seq));
	strand=ajSeqStrCopy(seq);

	ajStrAssSubC(&substr,ajStrStr(strand),begin-1,end-1);
	len = ajStrLen(substr);

	alen = len*mlen;
	if(alen>maxarr)
	{
	    AJCRESIZE(path,alen);
	    AJCRESIZE(compass,alen);
	    maxarr=alen;
	}

	ajStrAssC(&m,"");
	ajStrAssC(&n,"");

	scan_profile(substr,pname,name,mname,mlen,fmatrix,thresh,maxs,
		     gapopen,gapextend,outf,&cons,opencoeff,extendcoeff,
		     path,compass,&m,&n,len,begin);
	
	ajStrDel(&strand);    
    }

    for(i=0;i<mlen;++i)
	AJFREE (fmatrix[i]);
    AJFREE (fmatrix);
    
    ajStrDel(&line);
    ajStrDel(&name);
    ajStrDel(&mname);
    ajStrDel(&substr);
    ajSeqDel(&seq);
    ajFileClose(&inf);
    ajFileClose(&outf);
    ajExit();
    return 0;
}




int getType(AjPFile inf, AjPStr *tname)
{
    AjPStr line=NULL;
    char *p=NULL;
    int  ret=0;
    
    line=ajStrNew();

    while(ajFileReadLine(inf,&line))
    {
	p=ajStrStr(line);
	if(!*p || *p=='#' || *p=='!' || *p=='\n') continue;
	break;
    }

    if(!strncmp(p,"Gribskov",8)) ret=1;
    if(!strncmp(p,"Henikoff",8)) ret=2;

    ajStrAssC(tname,p);

    ajStrDel(&line);
    return ret;
}






static void read_profile(AjPFile inf, AjPStr *name, AjPStr *mname, int *mlen,
			  float *gapopen, float *gapextend, int *thresh,
			  float *maxs, AjPStr *cons)
{
    char *p;
    
    AjPStr line=NULL;

    line=ajStrNew();

    if(!ajFileReadLine(inf,&line))
	ajFatal("Premature EOF in profile file");
    p = ajStrStr(line);
    if(strncmp(p,"Name",4))
	ajFatal("Incorrect profile/matrix file format");
    p=strtok(p," \t");
    p=strtok(NULL," \t");
    ajStrAssC(name,p);

    if(!ajFileReadLine(inf,&line))
	ajFatal("Premature EOF in profile file");
    p = ajStrStr(line);
    if(strncmp(p,"Matrix",6))
	ajFatal("Incorrect profile/matrix file format");
    p=strtok(p," \t");
    p=strtok(NULL," \t");
    ajStrAssC(mname,p);


    if(!ajFileReadLine(inf,&line))
	ajFatal("Premature EOF in profile file");
    p = ajStrStr(line);
    if(strncmp(p,"Length",6))
	ajFatal("Incorrect profile/matrix file format");
    sscanf(p,"%*s%d",mlen);

    if(!ajFileReadLine(inf,&line))
	ajFatal("Premature EOF in profile file");
    p = ajStrStr(line);
    if(strncmp(p,"Max_score",9))
	ajFatal("Incorrect profile/matrix file format");
    sscanf(p,"%*s%f",maxs);

    if(!ajFileReadLine(inf,&line))
	ajFatal("Premature EOF in profile file");
    p = ajStrStr(line);
    if(strncmp(p,"Threshold",9))
	ajFatal("Incorrect profile/matrix file format");
    sscanf(p,"%*s%d",thresh);


    if(!ajFileReadLine(inf,&line))
	ajFatal("Premature EOF in profile file");
    p = ajStrStr(line);
    if(strncmp(p,"Gap_open",8))
	ajFatal("Incorrect profile/matrix file format");
    sscanf(p,"%*s%f",gapopen);

    if(!ajFileReadLine(inf,&line))
	ajFatal("Premature EOF in profile file");
    p = ajStrStr(line);
    if(strncmp(p,"Gap_extend",10))
	ajFatal("Incorrect profile/matrix file format");
    sscanf(p,"%*s%f",gapextend);

    if(!ajFileReadLine(inf,&line))
	ajFatal("Premature EOF in profile file");
    p = ajStrStr(line);
    if(strncmp(p,"Consensus",9))
	ajFatal("Incorrect profile/matrix file format");
    p=strtok(p," \t\n");
    p=strtok(NULL," \t\n");
    ajStrAssC(cons,p);



    ajStrDel(&line);
}





void scan_profile(AjPStr substr, AjPStr pname, AjPStr name, AjPStr mname,
		  int mlen, float **fmatrix, int thresh, float maxs,
		  float gapopen, float gapextend, AjPFile outf,
		  AjPStr *cons, float opencoeff, float extendcoeff,
		  float *path, int *compass, AjPStr *m, AjPStr *n,
		  int slen, int begin)
{
    float score;
    int start1;
    int start2;
    
    embAlignProfilePathCalc(ajStrStr(substr),mlen,slen,opencoeff,extendcoeff,
			    path,fmatrix,compass,0);
    
    score=embAlignScoreProfileMatrix(path,compass,opencoeff,extendcoeff,
				     substr,mlen,slen,fmatrix,&start1,
				     &start2);

    embAlignWalkProfileMatrix(path,compass,opencoeff,extendcoeff,*cons,
			      substr,m,n,mlen,slen,fmatrix,&start1,
			      &start2);
    
    embAlignPrintProfile(outf,ajStrStr(*cons),ajStrStr(substr),*n,*m,
			 start2,start1,score,1,fmatrix,ajStrStr(pname),
			 "Consensus",begin,1);

    return;
}

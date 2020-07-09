/* @source profit application
**
** Scan a protein database or sequence with a profile or matrix
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

#define AZ 27


static void read_profile(AjPFile inf, AjPStr *name, AjPStr *mname, ajint *mlen,
			  float *gapopen, float *gapextend, ajint *thresh,
			  float *maxs, AjPStr *cons);

static void read_simple(AjPFile inf, AjPStr *name, ajint *mlen,
			ajint *maxs, ajint *thresh,AjPStr *cons);

void scan_profile(AjPStr substr, AjPStr pname, AjPStr name, AjPStr mname,
		   ajint mlen, float **fmatrix, ajint thresh, float maxs,
		   float gapopen, float gapextend, AjPFile outf,
		   AjPStr *cons);

void scan_simple(AjPStr substr, AjPStr pname, AjPStr name,int mlen,int maxs,
		 ajint thresh,int **matrix,AjPFile outf,AjPStr *cons);

void printHits(AjPStr substr,AjPStr pname, ajint pos, AjPStr name, ajint score,
	       ajint thresh,float maxs, AjPFile outf,AjPStr *cons);

ajint getType(AjPFile inf);








int main(int argc, char **argv)
{
    AjPSeqall seqall;
    AjPSeq    seq=NULL;
    AjPFile   inf=NULL;
    AjPFile   outf=NULL;
    
    AjPStr    strand=NULL;
    AjPStr    substr=NULL;
    AjPStr    name=NULL;
    AjPStr    mname=NULL;
    AjPStr    pname=NULL;
    AjPStr    line=NULL;
    AjPStr    cons=NULL;
    
    ajint       type;
    ajint       begin;
    ajint       end;
    /*    ajint       len;*/
    ajint       i;
    ajint       j;
    
    ajint   **matrix=NULL;
    float **fmatrix=NULL;

    void  **fptr=NULL;
    
    ajint mlen;
    ajint maxs;
    float maxfs;
    ajint thresh;

    float gapopen;
    float gapextend;
    
    char *p;


    embInit("profit", argc, argv);

    seqall    = ajAcdGetSeqall("sequence");
    inf       = ajAcdGetInfile("infile");
    outf      = ajAcdGetOutfile("outfile");
    
    seq=ajSeqNew();
    substr=ajStrNew();
    name=ajStrNew();
    mname=ajStrNew();
    line=ajStrNew();

    if(!(type=getType(inf)))
	ajFatal("Unrecognised profile/matrix file format");
    
    switch(type)
    {
    case 1:
	read_simple(inf, &name, &mlen, &maxs, &thresh, &cons);
	AJCNEW(matrix, mlen);
	fptr=(void **)matrix;
    
	for(i=0;i<mlen;++i)
	{
	    AJCNEW(matrix[i], AZ);
	    if(!ajFileReadLine(inf,&line))
		ajFatal("Missing matrix line");
	    p = ajStrStr(line);
	    p = strtok(p," \t");
	    for(j=0;j<AZ;++j)
	    {
		sscanf(p,"%d",&matrix[i][j]);
		p = strtok(NULL," \t");
	    }
	}
	ajFmtPrintF(outf,"# PROF scan using simple frequency matrix %s\n",
		    ajStrStr(name));
	ajFmtPrintF(outf,"# Scores >= threshold %d (max score %d)\n#\n",thresh,
		    maxs);
	break;
    case 2:
	read_profile(inf,&name,&mname,&mlen,&gapopen,&gapextend,&thresh,
		      &maxfs, &cons);
	AJCNEW(fmatrix, mlen);
	fptr=(void **)fmatrix;
	for(i=0;i<mlen;++i)
	{
	    AJCNEW(fmatrix[i],(AZ+1));
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
	ajFmtPrintF(outf,"# PROF scan using Gribskov profile %s\n",
		    ajStrStr(name));
	ajFmtPrintF(outf,"# Scores >= threshold %d (max score %.2f)\n#\n",
		    thresh,maxfs);
	break;
    case 3:
	read_profile(inf,&name,&mname,&mlen,&gapopen,&gapextend,&thresh,
		      &maxfs, &cons);
	AJCNEW(fmatrix, mlen);
	fptr=(void **)fmatrix;
	for(i=0;i<mlen;++i)
	{
	    AJCNEW(fmatrix[i], (AZ+1));
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
	ajFmtPrintF(outf,"# PROF scan using Henikoff profile %s\n",
		    ajStrStr(name));
	ajFmtPrintF(outf,"# Scores >= threshold %d (max score %.2f)\n#\n",
		    thresh,maxfs);
	break;
    default:
	ajFatal("Switch type error");
	break;
    }
    
    
    while(ajSeqallNext(seqall, &seq))
    {
	begin=ajSeqallBegin(seqall);
	end=ajSeqallEnd(seqall);
	
	ajStrAssC(&pname,ajSeqName(seq));
	strand=ajSeqStrCopy(seq);

	ajStrAssSubC(&substr,ajStrStr(strand),begin-1,end-1);
	/*	len = ajStrLen(substr); NOT USED */
	switch(type)
	{
	case 1:
	    scan_simple(substr,pname,name,mlen,maxs,thresh,matrix,outf,&cons);
	    break;
	case 2:
	    scan_profile(substr,pname,name,mname,mlen,fmatrix,thresh,maxfs,
			  gapopen,gapextend,outf,&cons);
	    break;
	case 3:
	    scan_profile(substr,pname,name,mname,mlen,fmatrix,thresh,maxfs,
			  gapopen,gapextend,outf,&cons);
	    break;
	default:
	    break;
	}
	
	ajStrDel(&strand);    
    }

    for(i=0;i<mlen;++i)
	AJFREE (fptr[i]);
    AJFREE (fptr);
    
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




ajint getType(AjPFile inf)
{
    AjPStr line=NULL;
    char *p=NULL;
    ajint  ret=0;
    
    line=ajStrNew();

    while(ajFileReadLine(inf,&line))
    {
	p=ajStrStr(line);
	if(!*p || *p=='#' || *p=='!' || *p=='\n') continue;
	break;
    }

    if(!strncmp(p,"Simple",6)) ret=1;
    if(!strncmp(p,"Gribskov",8)) ret=2;
    if(!strncmp(p,"Henikoff",8)) ret=3;

    ajStrDel(&line);
    return ret;
}






static void read_simple(AjPFile inf, AjPStr *name, ajint *mlen,
		 ajint *maxs, ajint *thresh, AjPStr *cons)
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
    if(strncmp(p,"Length",6))
	ajFatal("Incorrect profile/matrix file format");
    sscanf(p,"%*s%d",mlen);

    if(!ajFileReadLine(inf,&line))
	ajFatal("Premature EOF in profile file");
    p = ajStrStr(line);
    if(strncmp(p,"Maximum",7))
	ajFatal("Incorrect profile/matrix file format");
    sscanf(p,"%*s%*s%d",maxs);

    if(!ajFileReadLine(inf,&line))
	ajFatal("Premature EOF in profile file");
    p = ajStrStr(line);
    if(strncmp(p,"Thresh",6))
	ajFatal("Incorrect profile/matrix file format");
    sscanf(p,"%*s%d",thresh);

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






static void read_profile(AjPFile inf, AjPStr *name, AjPStr *mname, ajint *mlen,
			  float *gapopen, float *gapextend, ajint *thresh,
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





void scan_simple(AjPStr substr, AjPStr pname, AjPStr name,int mlen,int maxs,
		 ajint thresh,int **matrix,AjPFile outf, AjPStr *cons)
{
    ajint len;
    ajint i;
    ajint j;
    ajint lim;
    char *p;

    ajint score;
    ajint sum;

    
    len = ajStrLen(substr);
    lim = len-mlen+1;
    p = ajStrStr(substr);
    
    for(i=0;i<lim;++i)
    {
	sum=0;
	for(j=0;j<mlen;++j)
	    sum += matrix[j][ajAZToInt(*(p+i+j))];
	score = sum * 100 / maxs;
	if(score >= thresh)
	    printHits(substr,pname,i,name,score,thresh,(float)maxs,outf,
		      cons);
    }
}




void printHits(AjPStr substr,AjPStr pname, ajint pos, AjPStr name, ajint score,
	       ajint thresh, float maxs, AjPFile outf, AjPStr *cons)
{

    ajFmtPrintF(outf,"%s %d Percentage: %d\n",ajStrStr(pname),pos+1,score);
}



void scan_profile(AjPStr substr, AjPStr pname, AjPStr name, AjPStr mname,
		   ajint mlen, float **fmatrix, ajint thresh, float maxs,
		   float gapopen, float gapextend, AjPFile outf,
		   AjPStr *cons)
{
    ajint len;
    ajint i;
    ajint j;
    ajint lim;
    char *p;

    float score;
    float sum;

    
    len = ajStrLen(substr);
    lim = len-mlen+1;
    p = ajStrStr(substr);
    
    for(i=0;i<lim;++i)
    {
	sum=0.0;
	for(j=0;j<mlen;++j)
	    sum += fmatrix[j][ajAZToInt(*(p+i+j))];
	score = sum * 100. / maxs;
	if((ajint)score >= thresh)
	    printHits(substr,pname,i,name,(ajint)score,thresh,maxs,outf,
		      cons);
    }
    return;
}

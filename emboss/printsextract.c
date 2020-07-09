/* @source printsextract application
**
** Displays and plots nucleic acid duplex melting temperatures
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
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>


#define AZ 28
#define DATANAME "PRINTS/prints.mat"


AjBool  prints_entry(AjPStr *s, AjPFile *fp);
void    write_code(AjPFile *fp, AjPStr *s, AjPStr *c);
void    write_accession(AjPFile *inf, AjPFile *outf, AjPStr *s, AjPStr *acc);
ajint     write_sets(AjPFile *inf, AjPFile *outf, AjPStr *s);
void    write_title(AjPFile *inf, AjPFile *outf, AjPStr *s);
void    write_desc(AjPFile *inf, AjPStr *s, AjPStr *acc, AjPStr *code);
void    skipToDn(AjPFile *inf, AjPStr *s);
ajlong    skipToFm(AjPFile *inf, AjPStr *s);
void    getSeqNumbers(AjPFile *inf, ajint *cnts, ajint *lens, AjPStr *s, ajint n);
void    calcMatrices(AjPFile *inf, AjPFile *outf, ajint *cnts, ajint *lens,
		     AjPStr *s, ajint n);
void    printHeader(AjPFile outf);




int main(int argc, char **argv)
{
    AjPFile inf  = NULL;
    AjPFile outf = NULL;
    AjPStr  pfname;
    AjPStr  line;
    AjPStr  acc;
    AjPStr  code;
    ajint nsets;

    ajlong pos;
    ajint *cnts=NULL;
    ajint *lens=NULL;
    
    embInit("printsextract",argc,argv);
    
    inf = ajAcdGetInfile("inf");
    pfname = ajStrNewC(DATANAME);
    ajFileDataNewWrite(pfname,&outf);
    printHeader(outf);
    
    line = ajStrNew();
    acc  = ajStrNew();
    code = ajStrNew();
    
    while(prints_entry(&line,&inf))
    {
	write_code(&outf,&line,&code);
	write_accession(&inf,&outf,&line,&acc);
	nsets = write_sets(&inf,&outf,&line);
	AJCNEW (cnts, nsets);
	AJCNEW (lens, nsets);
	write_title(&inf,&outf,&line);
	write_desc(&inf,&line,&acc,&code);
	skipToDn(&inf,&line);
	pos = skipToFm(&inf,&line);
        getSeqNumbers(&inf, cnts, lens, &line, nsets);
	ajFileSeek(inf,pos,SEEK_SET);
	calcMatrices(&inf, &outf, cnts, lens, &line, nsets);
	AJFREE (lens);
	AJFREE (cnts);
	ajFmtPrintF(outf,"//\n");
    }
    ajStrDel(&line);
    ajStrDel(&acc);
    ajStrDel(&code);
    ajExit();
    return 0;
}




AjBool prints_entry(AjPStr *s, AjPFile *fp)
{
    while(ajFileReadLine(*fp,s))
	if(ajStrPrefixC(*s,"gc;"))
	    return ajTrue;
    return ajFalse;
}


void write_code(AjPFile *fp, AjPStr *s, AjPStr *c)
{
    char *p;

    p=ajStrStr(*s)+4;
    ajStrAssC(c,p);
    ajDebug("Reading code %s\n",p);
    ajFmtPrintF(*fp,"%s\n",p);
}



void write_accession(AjPFile *inf, AjPFile *outf, AjPStr *s, AjPStr *a)
{
    char *p;
    
    if(!ajFileReadLine(*inf,s)) ajFatal("Premature EOF");
    if(!ajStrPrefixC(*s,"gx;")) ajFatal("No accnum (%s)",ajStrStr(*s));
    ajStrChomp(s);
    p = ajStrStr(*s)+4;
    p = strchr(p,';');
    if(p)
	*p = '\0';
    ajStrAssC(a,ajStrStr(*s)+4);
    ajFmtPrintF(*outf,"%s\n",ajStrStr(*s)+4);
}


ajint write_sets(AjPFile *inf, AjPFile *outf, AjPStr *s)
{
    char *p;
    ajint n;
    
    if(!ajFileReadLine(*inf,s)) ajFatal("Premature EOF");
    if(!ajStrPrefixC(*s,"gn;")) ajFatal("No gn; (%s)",ajStrStr(*s));
    p=ajStrStr(*s);
    n=1;
    p=strchr(p,(ajint)'(');
    if(!p) ajFmtPrintF(*outf,"1\n");
    else 
    {
	sscanf(p+1,"%d",&n);
	ajFmtPrintF(*outf,"%d\n",n);
    }
    return n;
}



void write_title(AjPFile *inf, AjPFile *outf, AjPStr *s)
{
    if(!ajFileReadLine(*inf,s)) ajFatal("Premature EOF");
    if(!ajFileReadLine(*inf,s)) ajFatal("Premature EOF");
    if(!ajStrPrefixC(*s,"gt;")) ajFatal("No title (%s)",ajStrStr(*s));
    ajFmtPrintF(*outf,"%s\n",ajStrStr(*s)+4);
}
    

void write_desc(AjPFile *inf, AjPStr *s, AjPStr *a, AjPStr *c)
{
    AjPFile fp;
    AjPStr  fname;
    AjBool  e;
    
    fname = ajStrNewC("PRINTS/");
    ajStrApp(&fname,*a);
    ajFileDataNewWrite(fname,&fp);
    ajFmtPrintF(fp,"%s\n",ajStrStr(*c));
    while((e=ajFileReadLine(*inf,s)))
	if(ajStrPrefixC(*s,"gd;")) break;
    if(!e) ajFatal("Premature EOF");
    
    while(ajStrPrefixC(*s,"gd;"))
    {
	if(ajStrLen(*s)<5) ajFmtPrintF(fp,"\n");
	else
	    ajFmtPrintF(fp,"%s\n",ajStrStr(*s)+4);
	if(!ajFileReadLine(*inf,s))
	    ajFatal("Premature EOF");
    }
    ajFileClose(&fp);
    ajStrDel(&fname);
    return;
}


void skipToDn(AjPFile *inf, AjPStr *s)
{
    while(ajFileReadLine(*inf,s))
    {
	if(ajStrPrefixC(*s,"gc;")) ajFatal("Missing dn; line");
	if(ajStrPrefixC(*s,"dn;")) return;
    }
    ajFatal("Premature EOF");
}


ajlong skipToFm(AjPFile *inf, AjPStr *s)
{
    while(ajFileReadLine(*inf,s))
    {
	if(ajStrPrefixC(*s,"gc;")) ajFatal("Missing fm; line");
	if(ajStrPrefixC(*s,"fm;"))
	    return ajFileTell(*inf);
    }
    ajFatal("Premature EOF");
    return 0L;
}


void getSeqNumbers(AjPFile *inf, ajint *cnts, ajint *lens, AjPStr *s, ajint n)
{
    ajint i;
    ajint c;
    ajint len;
    
    for(i=0;i<n;++i)
    {
	while(!ajStrPrefixC(*s,"fl;"))
	{
	    if(!ajFileReadLine(*inf,s))
		ajFatal("Premature EOF");
	    if(ajStrPrefixC(*s,"gc;"))
		ajFatal("Missing fl; line");
	}
	sscanf(ajStrStr(*s)+4,"%d",&len);
	lens[i]=len;
	ajFileReadLine(*inf,s);
	ajFileReadLine(*inf,s);
	if(!ajStrPrefixC(*s,"fd;"))
	    ajFatal("Missing fd; line (%s)\n",ajStrStr(*s));

	c=0;
	while(ajStrPrefixC(*s,"fd;"))
	{
	    ++c;
	    if(!ajFileReadLine(*inf,s))
		ajFatal("Premature EOF");
	    if(ajStrPrefixC(*s,"gc;"))
		ajFatal("Missing fl; line");
	}
	cnts[i]=c;
    }
}



void calcMatrices(AjPFile *inf, AjPFile *outf, ajint *cnts, ajint *lens,
		  AjPStr *s, ajint n)
{
    ajint i;
    ajint j;
    ajint k;
    ajint c;
    static ajint *mat[AZ];
    char *p;
    ajint sum;
    ajint max;
    ajint min;
    ajint fmin;
    ajlong pos;
    ajint  t;
    
    for(i=0;i<n;++i)
    {
	pos = ajFileTell(*inf);
	while(!ajStrPrefixC(*s,"fd;"))
	{
	    if(!ajFileReadLine(*inf,s))
		ajFatal("Premature EOF");
	}
	ajFmtPrintF(*outf,"%d\n",lens[i]);

	c=lens[i];
	for(j=0;j<AZ;++j)
	    AJCNEW0(mat[j], c);


	for(j=0;j<cnts[i];++j)
	{
	    p=ajStrStr(*s)+4;
	    for(k=0;k<c;++k)
	    {
		t = ajAZToInt(*(p+k));
		if(t>AZ)
		    printf("Error\n");
		++mat[ajAZToInt(*(p+k))][k];
	    }
	    
	    ajFileReadLine(*inf,s);
	}


	for(j=0,sum=0;j<c;++j)
	{
	    for(k=0,max=0,fmin=0;k<AZ;++k)
		max = (max>mat[k][j]) ? max : mat[k][j];
	    sum += max;
	}


	ajFileSeek(*inf,pos,SEEK_SET);
	ajFileReadLine(*inf,s);
	
	while(!ajStrPrefixC(*s,"fd;"))
	{
	    if(!ajFileReadLine(*inf,s))
		ajFatal("Premature EOF");
	}
	for(j=0,min=INT_MAX;j<cnts[i];++j)
	{
	    fmin=0;
	    p=ajStrStr(*s)+4;
	    for(k=0;k<c;++k)
		{
		    
		    fmin+=mat[ajAZToInt(*(p+k))][k];
		}
	    
	    min=(min<fmin) ? min : fmin;
	    ajFileReadLine(*inf,s);
	}

	ajFmtPrintF(*outf,"%d\n%d\n",(min*100)/sum,sum);


	for(j=0;j<26;++j)
	{
	    for(k=0;k<c;++k)
		ajFmtPrintF(*outf,"%2d ",mat[j][k]);
	    ajFmtPrintF(*outf,"\n");
	}

	for(j=0;j<AZ;++j)
	    AJFREE (mat[j]);
    }
}




void printHeader(AjPFile outf)
{
    ajFmtPrintF(outf,"# PRINTS matrix file for EMBOSS extracted from prints.dat\n");
    
    ajFmtPrintF(outf,"# Format:\n");
    ajFmtPrintF(outf,"# Line 1: PRINTS code\n");
    ajFmtPrintF(outf,"# Line 2: PRINTS accession number\n");
    ajFmtPrintF(outf,"# Line 3: Number of elements in the fingerprint\n");
    ajFmtPrintF(outf,"# Line 4: Description of fingerprint\n");
    ajFmtPrintF(outf,"# Then, for each element\n");
    ajFmtPrintF(outf,"# Line 5: Length of element\n");
    ajFmtPrintF(outf,"# Line 6: Threshold %% in order to match\n");
    ajFmtPrintF(outf,"# Line 7: Maximum score for this matrix\n");
    ajFmtPrintF(outf,"# The matrix is then on subsequent lines ...\n");
    ajFmtPrintF(outf,"# A  n1 n2 n3 n4 ....\n");
    ajFmtPrintF(outf,"# B  n1 n2 n3 n4 ....\n");
    ajFmtPrintF(outf,"# ..\n");
    ajFmtPrintF(outf,"# Z  n1 n2 n3 n4 ....\n");
    ajFmtPrintF(outf,"#\n");
}

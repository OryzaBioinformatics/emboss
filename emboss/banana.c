/* @source banana application
**
** Banana displays Bending and Curvature Calculations.
**
** @author: Copyright (C) Ian Longden (il@sanger.ac.uk)
** @@
** please reference the following report in any publication resulting from
** use of this program.
** Goodsell, D.S. & Dickerson, R.E. (1994) "Bending and Curvature Calculations
** in B-DNA" Nucl. Acids. Res. 22, 5497-5503.
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


#define sinf(x) (float)sin((double)x)
#define cosf(x) (float)cos((double)x)








/* @prog banana ***************************************************************
**
** Bending and curvature plot in B-DNA
**
******************************************************************************/

int main(int argc, char **argv)
{
    AjPSeq seq;
    AjPGraph graph = 0;
    AjPFile   outf=NULL;
    AjPFile file = NULL;
    AjPStr buffer = NULL;
    float twist[4][4][4];
    float roll[4][4][4];
    float tilt[4][4][4];
    float rbend;
    float rcurve;
    float bendscale;
    float curvescale;
    float twistsum=0.0;
    float pi    = 3.14159;
    float pifac = (pi/180.0);
    float pi2   = pi/2.0;
    ajint *iseq   = NULL;
    float *x;
    float *y;
    float *xave;
    float *yave;
    float *curve;
    float *bend;
    char *ptr;
    ajint i;
    ajint k;
    ajint j;
    char residue[2];
    float maxbend;
    float minbend;
    float bendfactor;
    float maxcurve;
    float mincurve;
    float curvefactor;

    float fxp,fyp,yincr,y1;
    ajint ixlen;
    ajint iylen;
    ajint ixoff;
    ajint iyoff;
    float ystart;
    float defheight;
    float currentheight;
    ajint count;
    ajint portrait = 0,title=0;
    ajint numres;
    ajint ibeg;
    ajint iend;
    ajint ilen;
    AjPStr  sstr  = NULL;
    AjPFile goutf = NULL;
    AjPStr  fname = NULL;
    ajint     fno=1;
    AjBool  data;
    


    ajGraphInit ("banana", argc, argv);
    seq    = ajAcdGetSeq ("sequence");
    file   = ajAcdGetDatafile("anglesfile");
    outf   = ajAcdGetOutfile("outfile");
    graph  = ajAcdGetGraph("graph");
    numres = ajAcdGetInt("residuesperline");
    data   = ajAcdGetBool("data");
    



    ibeg = ajSeqBegin(seq);
    iend = ajSeqEnd(seq);

    ajStrAssSub (&sstr, ajSeqStr(seq), ibeg-1, iend-1);
    ilen = ajStrLen(sstr);

    AJCNEW0(iseq,  ilen+1);
    AJCNEW0(x,     ilen+1);
    AJCNEW0(y,     ilen+1);
    AJCNEW0(xave,  ilen+1);
    AJCNEW0(yave,  ilen+1);
    AJCNEW0(curve, ilen+1);
    AJCNEW0(bend,  ilen+1);

    ptr= ajStrStr(sstr);

    for(i=0;i<ajStrLen(sstr);i++)
    {
	if(*ptr=='A' || *ptr=='a')
	    iseq[i+1] = 0;
	else if(*ptr=='T' || *ptr=='t')
	    iseq[i+1] = 1;
	else if(*ptr=='G' || *ptr=='g')
	    iseq[i+1] = 2;
	else if(*ptr=='C' || *ptr=='c')
	    iseq[i+1] = 3;
	else
	    ajErr("%c is not an ATCG hence not valid",*ptr);
	ptr++;
    }


    if(!file)
	ajErr("EMBOSS_DATA undefined");
  
    ajFileGets(file,&buffer);		/* 3 junk lines */
    ajFileGets(file,&buffer);
    ajFileGets(file,&buffer);

    for(k=0;k<4;k++)
    {
	for(i=0;i<4;i++)
	{
	    if(ajFileGets(file,&buffer))
	    {
		sscanf(ajStrStr(buffer),"%f,%f,%f,%f",
		       &twist[i][0][k],&twist[i][1][k],&twist[i][2][k],
		       &twist[i][3][k]);	
	    }
	    else
		ajErr("Error reading angle file");
	    for(j=0;j<4;j++)
		twist[i][j][k] *= pifac;
	}
    }

    for(k=0;k<4;k++)
    {
	for(i=0;i<4;i++)
	{
	    if(ajFileGets(file,&buffer))
	    {
		sscanf(ajStrStr(buffer),"%f,%f,%f,%f",&roll[i][0][k],
		       &roll[i][1][k],&roll[i][2][k],&roll[i][3][k]);
	    }
	    else
		ajErr("Error reading angle file");
	}
    }

    for(k=0;k<4;k++)
    {
	for(i=0;i<4;i++)
	{
	    if(ajFileGets(file,&buffer))
		sscanf(ajStrStr(buffer),"%f,%f,%f,%f",&tilt[i][0][k],
		       &tilt[i][1][k],&tilt[i][2][k],&tilt[i][3][k]);
	    else
		ajErr("Error reading angle file");
	}
    }


  
    if(ajFileGets(file,&buffer))
	sscanf(ajStrStr(buffer),"%f,%f,%f,%f",&rbend,&rcurve,
	       &bendscale,&curvescale);
    else
	ajErr("Error reading angle file");
  
    ajFileClose(&file);
  
  
    for(i=1;i<ajStrLen(sstr)-1;i++)
    {
	float dx,dy;

	twistsum += twist[iseq[i]][iseq[i+1]][iseq[i+2]];
	dx = (roll[iseq[i]][iseq[i+1]][iseq[i+2]]*sinf(twistsum)) +
	    (tilt[iseq[i]][iseq[i+1]][iseq[i+2]]*sinf(twistsum-pi2));
	dy = roll[iseq[i]][iseq[i+1]][iseq[i+2]]*cosf(twistsum) +
	    tilt[iseq[i]][iseq[i+1]][iseq[i+2]]*cosf(twistsum-pi2);
 
	x[i+1] = x[i]+dx;   
	y[i+1] = y[i]+dy;

    }

    for(i=6;i<ajStrLen(sstr)-6;i++)
    {
	float rxsum,rysum;

	rxsum = 0.0;
	rysum = 0.0;
	for(k=-4;k<=4;k++)
	{
	    rxsum+=x[i+k];
	    rysum+=y[i+k];
	}
	rxsum+=(x[i+5]*0.5);
	rysum+=(y[i+5]*0.5);
	rxsum+=(x[i-5]*0.5);
	rysum+=(y[i-5]*0.5);

	xave[i] = rxsum*0.1;
	yave[i] = rysum*0.1;
    }

    for(i=(ajint)rbend+1;i<=ajStrLen(sstr)-(ajint)rbend-1;i++)
    {
	bend[i] = sqrt(((x[i+(ajint)rbend]-x[i-(ajint)rbend])*
			(x[i+(ajint)rbend]-x[i-(ajint)rbend])) +
		       ((y[i+(ajint)rbend]-y[i-(ajint)rbend])*
			(y[i+(ajint)rbend]-y[i-(ajint)rbend])));
	bend[i]*=bendscale;
    }

    for(i=(ajint)rcurve+6;i<=ajStrLen(sstr)-(ajint)rcurve-6;i++)
    {
	curve[i] = sqrt(((xave[i+(ajint)rcurve]-
			  xave[i-(ajint)rcurve])*(xave[i+(ajint)rcurve]-
						xave[i-(ajint)rcurve]))+
			((yave[i+(ajint)rcurve]-yave[i-(ajint)rcurve])*
			 (yave[i+(ajint)rcurve]-yave[i-(ajint)rcurve])));
    }
 

    ajFmtPrintF(outf,"Base   Bend      Curve\n");
    ptr= ajStrStr(sstr);
    for(i=1;i<=ajStrLen(sstr);i++)
    {
	ajFmtPrintF(outf,"%c    %6.1f   %6.1f\n",*ptr, bend[i], curve[i]);
	ptr++;
    } 
    ajFileClose(&outf);

    maxbend = minbend = 0.0;
    maxcurve = mincurve = 0.0;
    for(i=1;i<=ajStrLen(sstr);i++)
    {
	if(bend[i] > maxbend)
	    maxbend = bend[i];
	if(bend[i] < minbend)
	    minbend = bend[i];
	if(curve[i] > maxcurve)
	    maxcurve = curve[i];
	if(curve[i] < mincurve)
	    mincurve = curve[i];
    }

    ystart = 75.0;

    if(!data)
	ajGraphOpenWin(graph,-1.0, (float)numres+10.0, 0.0, ystart+5.0);
    else
    {
	ajFmtPrintS(&fname,"banana%d.dat",fno++);
	if(!(goutf=ajFileNewOut(fname)))
	    ajFatal("Cannot open file %S",fname);
	ajUser("Writing to file %S",fname);
	ajFmtPrintF(goutf,"##Graphic\n##Screen x1 %f y1 %f x2 %f y2 %f\n",
		    -1.0,0.0,(float)numres+10.0,ystart+5.0);
    }
    

    if(!data)
	ajGraphTextMid ((numres+10.0)/2.0, ystart+2.5,ajStrStr(graph->title));
    else
	ajFmtPrintF(goutf,"Text2 x1 %f y1 %f colour 0 size 1.0 %s\n",
		    (numres+10.0)/2.0,ystart+2.5,ajSeqName(seq));
    
    if(!data)
	ajGraphGetOut(&fxp,&fyp,&ixlen,&iylen,&ixoff,&iyoff);
    else
    {
	fxp=fyp=0.;
	ixlen=600;
	iylen=450;
	ixoff=iyoff=0;
    }
    
    /*ajUser("%f\n%f\n%d\n%d\n%d\n%d",fxp,fyp,ixlen,iylen,ixoff,iyoff);*/
  
    if(ixlen == 0.0)
    {	/* for postscript these are 0.0 ????? */
	if(portrait)
	{
	    ixlen = 768;
	    iylen = 960;
	}      
	else{
	    ixlen = 960;
	    iylen = 768;
	}
    }

    if(!data)
    {
	ajGraphGetCharSize(&defheight,&currentheight);
	ajGraphSetCharSize(((float)ixlen/((float)(numres)*
					  (currentheight+1.0)))/currentheight);
	ajGraphGetCharSize(&defheight,&currentheight);
    }
    else
    {
	defheight = currentheight = 4.440072;
	currentheight = defheight * ((float)ixlen/
				     ((float)(numres)*(currentheight+1.0)))
	    /currentheight;
    }
    
  
    yincr = (currentheight +3.0)*0.3;
  
    if(!title)
	y1=ystart;
    else
    {
	y1=ystart-5.0;
    }
    count = 1;

    residue[1]='\0';

    bendfactor = (3*yincr)/maxbend;
    curvefactor = (3*yincr)/maxcurve;

    ptr= ajStrStr(sstr);

    y1=y1-(yincr*(5.0));
    for(i=1;i<=ajStrLen(sstr);i++)
    {
	if(count > numres)
	{
	    y1=y1-(yincr*(5.0));
	    if(y1<1.0)
	    {
		if(!title)
		    y1=ystart;
		else
		{
		    y1=ystart-5.0;
		}
		y1=y1-(yincr*(5.0));
		if(!data)
		    ajGraphNewPage(AJFALSE);
		else
		{
		    ajFileClose(&goutf);
		    ajFmtPrintS(&fname,"banana%d.dat",fno++);
		    if(!(goutf=ajFileNewOut(fname)))
			ajFatal("Cannot open file %S",fname);
		    ajUser("Writing to file %S",fname);
		    ajFmtPrintF(goutf,"##Graphic\n##Screen x1 %f y1 %f x2"
				" %f y2 %f\n",
				-1.0,0.0,(float)numres+10.0,ystart+5.0);
		}
	    }
	    count=1;
	}
	residue[0] = *ptr;

	if(!data)
	    ajGraphTextEnd ((float)(count)+2.0,y1,residue);
	else
	    ajFmtPrintF(goutf,"Text3 x1 %f y1 %f colour 0 size %f %s\n",
			(float)(count)+2.0,y1,currentheight,residue);
	
	if(i>1 && i < ajStrLen(sstr))
	{
	    float yp1,yp2;
	    yp1=y1+yincr + (bend[i]*bendfactor);
	    yp2=y1+yincr + (bend[i+1]*bendfactor);
	    if(!data)
		ajGraphLine((float)count+1.5,yp1,(float)(count)+2.5,yp2);
	    else
		ajFmtPrintF(goutf,"Line x1 %f y1 %f x2 %f y2 %f colour 0\n",
			    (float)count+1.5,yp1,(float)(count)+2.5,yp2);
	    
	}
	if(i>(ajint)rcurve+5 && i< ajStrLen(sstr)-(ajint)rcurve-7)
	{
	    float yp1,yp2;
	    yp1=y1+yincr + (curve[i]*curvefactor);
	    yp2=y1+yincr + (curve[i+1]*curvefactor);
	    if(!data)
		ajGraphLine((float)count+1.7,yp1,(float)(count)+2.3,yp2);
	    else
		ajFmtPrintF(goutf,"Line x1 %f y1 %f x2 %f y2 %f colour 0\n",
			    (float)count+1.7,yp1,(float)(count)+2.3,yp2);
	    
	}
	if(!data)
	    ajGraphLine((float)count+1.5,y1+yincr,(float)(count)+2.5,y1+yincr);
	else
	    ajFmtPrintF(goutf,"Line x1 %f y1 %f x2 %f y2 %f colour 0\n",
			(float)count+1.5,y1+yincr,(float)(count)+2.5,y1+yincr);
      
	count++;
	ptr++;
    } 
 
    if(!data)
	ajGraphCloseWin();
    else
	ajFileClose(&goutf);


    AJFREE(iseq);
    AJFREE(x);
    AJFREE(y);
    AJFREE(xave);
    AJFREE(yave);
    AJFREE(curve);
    AJFREE(bend);

    ajExit();
    return 0;
}


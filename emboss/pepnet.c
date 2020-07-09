/* @source pepnet application
**
** Displays proteins as a helical net
** @author: Copyright (C) Alan Bleasby (ableasby@hgmp.mrc.ac.uk)
** @@
**
** Original program "PEPNET" in EGCG
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
#include <string.h>



static void plotresidue(char c, float x, float y, char *squares, char *circles,
			char *diamonds, AjBool text, AjPFile outf);
static void drawocta(float x, float y, float size, AjBool text,
		     AjPFile outf);




int main(int argc, char **argv)
{
    AjPSeq    seq=NULL;
    AjPStr    strand=NULL;
    AjPStr    substr=NULL;
    AjPStr    squares=NULL;
    AjPStr    diamonds=NULL;
    AjPStr    octags=NULL;
    AjBool    amphipathic;
    AjPStr    txt=NULL;
    AjPGraph  graph=NULL;

    ajint begin;
    ajint end;

    ajint lc;
    
    ajint i;
    ajint j;
    ajint r;
    
    ajint count;
    ajint pstart;
    ajint pstop;
    
    float xmin=   0.0;
    float xmax= 150.0;
    float ymin=   0.0;
    float ymax= 112.5;

    float xstart;
    float ystart;
    float ch     =   1.8;
    float xinc;
    float yinc;
    AjBool text;
    AjPFile outf=NULL;
    ajint fno;
    AjPStr fstr=NULL;
    


    float x;
    float y;
    

    (void) ajGraphInit("pepnet", argc, argv);


    seq         = ajAcdGetSeq("sequence");
    graph       = ajAcdGetGraph("graph");
    octags      = ajAcdGetString("octags");
    squares     = ajAcdGetString("squares");
    diamonds    = ajAcdGetString("diamonds");
    amphipathic = ajAcdGetBool("amphipathic");
    text        = ajAcdGetBool("data");
    
    ajStrToUpper(&octags);
    ajStrToUpper(&squares);
    ajStrToUpper(&diamonds);

    if(amphipathic)
    {
	ajStrAssC(&squares,"ACFGILMVWY");
	ajStrAssC(&diamonds,"");
	ajStrAssC(&octags,"");
    }


    substr = ajStrNew();
    txt    = ajStrNew();
    fno    = 1;
    fstr   = ajStrNew();
    

    
    
    begin=ajSeqBegin(seq);
    end=ajSeqEnd(seq);
	
    strand = ajSeqStrCopy(seq);
	
    ajStrToUpper(&strand);
    ajStrAssSubC(&substr,ajStrStr(strand),begin-1,end-1);

    ajGraphSetBackWhite();


    if(!text)
	ajGraphOpenWin(graph, xmin,xmax,ymin,ymax);
    else
    {
	ajFmtPrintS(&fstr,"pepnet%d.dat",fno++);
	if(!(outf = ajFileNewOut(fstr)))
	    ajFatal("Cannot open file %S",fstr);
	ajUser("Writing to file %S",fstr);
	ajFmtPrintF(outf,"##Graphic\n##Screen x1 %f y1 %f x2 %f y2 %f\n",
		    xmin,ymin,xmax,ymax);
    }
    

    for(count=begin-1,r=0;count<end;count+=231)
    {
	pstart=count;
	pstop = AJMIN(end-1, count+230);
	    
	ajFmtPrintS(&txt,"PEPNET of %s from %d to %d",ajSeqName(seq),
		    pstart+1,pstop+1);


	if(!text)
	{
	    ajGraphSetFore(RED);
	    ajGraphText(75.0,110.0,ajStrStr(txt),0.5);
	    ajGraphSetCharSize(0.75);
	}
	else
	    ajFmtPrintF(outf,"Text1 x1 %f y1 %f colour %d size 0.5 %s\n",
			75.,110.,RED,ajStrStr(txt));
	
	
	
	xstart = 145.0;
	ystart =  80.0;

	yinc = ch * 2.5;
	xinc = yinc / 2.5;
	
	x = xstart;
	
	for(i=pstart;i<=pstop;i+=7)
	{
	    lc = i;
	    if(x < 10.0*xinc)
	    {
		x = xstart;
		ystart -= 7.5*yinc;
	    }
	    y=ystart;
	    
	    ajFmtPrintS(&txt,"%d",i+1);

	    if(!text)
	    {
		ajGraphSetFore(RED);
		ajGraphText(x-xinc,y-yinc-1,ajStrStr(txt),0.5);
	    }
	    else
		ajFmtPrintF(outf,"Text1 x1 %f y1 %f colour %d size 0.5 %s\n",
			    x-xinc,y-yinc-1,RED,ajStrStr(txt));


	    for(j=0;j<4;++j)
	    {
		x -= xinc;
		y += yinc;
		if(lc <= pstop)
		    plotresidue(*(ajStrStr(substr)+r),x,y,
				ajStrStr(squares),ajStrStr(octags),
				ajStrStr(diamonds),text,outf);
		++r;
		++lc;
	    }
	    y=ystart+yinc/2.0;
	    for(j=4;j<7;++j)
	    {
		x -= xinc;
		y += yinc;
		if(lc <= pstop)
		    plotresidue(*(ajStrStr(substr)+r),x,y,
				ajStrStr(squares),ajStrStr(octags),
				ajStrStr(diamonds),text,outf);
		++r;
		++lc;
	    }
	}

	if(!text)
	    ajGraphNewPage(1);
	else
	{
	    ajFileClose(&outf);
	    if(count+231<end)
	    {
		ajFmtPrintS(&fstr,"pepnet%d.dat",fno++);
		if(!(outf = ajFileNewOut(fstr)))
		    ajFatal("Cannot open file %S",fstr);
		ajUser("Writing to file %S",fstr);
		ajFmtPrintF(outf,"##Graphic\n##Screen x1 %f y1 %f x2 %f "
			    "y2 %f\n",xmin,ymin,xmax,ymax);
	    }
	    
	}
	
    }

    if(!text)
	ajGraphCloseWin();
    else
	ajFileClose(&outf);
    
    ajStrDel(&strand);
    ajStrDel(&fstr);
    

    

    ajSeqDel(&seq);
    ajStrDel(&substr);
    
    ajExit();
    return 0;
}



static void drawocta(float x, float y, float size, AjBool text,
		     AjPFile outf)
{
    static float polyx[]=
    {
	-0.05, 0.05, 0.1, 0.1, 0.05, -0.05, -0.1, -0.1, -0.05
    };
    static float polyy[]=
    {
	0.1, 0.1, 0.05, -0.05, -0.1, -0.1, -0.05, 0.05, 0.1
    };

    ajint i;

    for(i=0;i<8;++i)
    {
	if(!text)
	    ajGraphLine(x+polyx[i]*size,y+polyy[i]*size,x+polyx[i+1]*size,
			y+polyy[i+1]*size);
	else
	    ajFmtPrintF(outf,"Line x1 %f y1 %f x2 %f y2 %f colour %d\n",
			x+polyx[i]*size,y+polyy[i]*size,x+polyx[i+1]*size,
			y+polyy[i+1]*size,BLUEVIOLET);
    }
    
    return;
}


static void plotresidue(char c, float x, float y, char *squares, char *octags,
		 char *diamonds, AjBool text, AjPFile outf)
{
    static char cs[2];

    cs[1]='\0';
    *cs=c;
    
    
    if(!text)
	ajGraphSetFore(GREEN);

    if(strstr(squares,cs))
    {
	if(!text)
	{
	    ajGraphSetFore(BLUE);
	    ajGraphBox(x-1.5,y-1.32,3.0);
	}
	else
	    ajFmtPrintF(outf,"Rectangle x1 %f y1 %f x2 %f y2 %f colour %d\n",
			x-1.5,y-1.32,x+1.5,y+1.68,BLUE);
    }
    if(strstr(octags,cs))
    {
	if(!text)
	    ajGraphSetFore(BLUEVIOLET);
	drawocta(x,y+0.225,20.0,text,outf);
    }
    if(strstr(diamonds,cs))
    {
	if(!text)
	{
	    ajGraphSetFore(RED);
	    ajGraphDia(x-2.5,y-2.25,5.0);
	}
	else
	    ajFmtPrintF(outf,"Shaded Rectangle x1 %f y1 %f x2 %f y2 %f "
			"colour %d\n",
			x-1.5,y-1.32,x+1.5,y+1.68,RED);

    }

    if(!text)
    {
	ajGraphText(x,y,cs,0.5);
	ajGraphSetFore(GREEN);
    }
    else
	ajFmtPrintF(outf,"Text1 x1 %f y1 %f colour %d size 0.5 %s\n",
		    x,y,GREEN,cs);
    return;
}

/* @source pepwheel application
**
** Displays peptide sequences in a helical representation
** @author: Copyright (C) Alan Bleasby (ableasby@hgmp.mrc.ac.uk)
** @@
** Original program "PEPWHEEL" by Rodrigo Lopez (EGCG)
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
#include <string.h>



static void pepwheel_plotresidue(char c, float r, float a, char *squares,
				 char *circles, char *diamonds, AjBool text,
				 AjPFile outf, float xmin,float xmax,
				 float ymin,float ymax);
static void pepwheel_drawocta(float x, float y, float size, AjBool text,
			      AjPFile outf);



#define AJB_BLUE   9
#define AJB_BLACK  0
#define AJB_GREY   7
#define AJB_RED    1
#define AJB_PURPLE 10



/* @prog pepwheel *************************************************************
**
** Shows protein sequences as helices
**
******************************************************************************/

int main(int argc, char **argv)
{
    AjPSeq    seq=NULL;
    AjPStr    strand=NULL;
    AjPStr    substr=NULL;
    AjPStr    squares=NULL;
    AjPStr    diamonds=NULL;
    AjPStr    octags=NULL;
    AjBool    wheel;
    AjBool    amphipathic;
    AjPStr    txt=NULL;
    AjPGraph  graph=0;
    AjBool    text;
    AjPFile   outf=NULL;
    AjBool first;
    AjBool startloop;

    ajint begin;
    ajint end;
    ajint len;

    ajint steps;
    ajint turns;
    ajint lc;

    ajint i;
    ajint j;
    ajint k;

    float xmin= -1.0;
    float xmax=  1.0;
    float ymin= -0.75;
    float ymax=  0.75;

    float minresplot= 36.0;
    float resgap=      0.0533;
    float wheelgap=    0.00;
    float nresgap=     0.08;




    float angle;
    float oldangle;
    float ang;
    float radius;
    float wradius;
    float x1;
    float x2;
    float y1;
    float y2;


    (void) ajGraphInit("pepwheel", argc, argv);


    seq       = ajAcdGetSeq("sequence");
    steps     = ajAcdGetInt("steps");
    turns     = ajAcdGetInt("turns");
    graph     = ajAcdGetGraph("graph");
    octags    = ajAcdGetString("octags");
    squares   = ajAcdGetString("squares");
    diamonds  = ajAcdGetString("diamonds");
    wheel     = ajAcdGetBool("wheel");
    amphipathic = ajAcdGetBool("amphipathic");
    text        = ajAcdGetBool("data");
    outf        = ajAcdGetOutfile("outfile");

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


    begin=ajSeqBegin(seq);
    end=ajSeqEnd(seq);
    ajDebug("begin: %d end: %d\n", begin, end);
    strand = ajSeqStrCopy(seq);

    ajStrToUpper(&strand);
    ajStrAssSubC(&substr,ajStrStr(strand),begin-1,end-1);
    len    = ajStrLen(substr);

    ajFmtPrintS(&txt,"PEPWHEEL of %s from %d to %d",ajSeqName(seq),
		begin,end);

    if(!text)
    {
	ajGraphOpenWin(graph,xmin,xmax,ymin,ymax);

	ajGraphSetFore(AJB_BLACK);
	ajGraphText(0.0,0.64,ajStrStr(txt),0.5);
	/*	ajGraphSetBackBlack();*/
	ajGraphSetFore(AJB_BLACK);
    }
    else
    {
	ajFmtPrintF(outf,"##Graphic\n##Screen x1 %f y1 %f x2 %f y2 %f\n",
		    xmin,ymin,xmax,ymax);
	ajFmtPrintF(outf,"Text1 x1 %f y1 %f colour 0 size 0.5 %s\n",
		    0.,0.64,ajStrStr(txt));
    }


    ang = (360.0 / (float)steps) * (float)turns;

    first = ajTrue;
    angle = 90.0 + ang;
    if(end-begin > (ajint)minresplot) wradius = 0.2;
    else                            wradius = 0.40;

    for(i=0,lc=0,radius=wradius+wheelgap;i<len;i+=steps)
    {
	wradius += wheelgap;
	startloop = ajTrue;
	k = AJMIN(i+steps, end);
	for(j=i;j<k;++j)
	{
	    oldangle=angle;
	    angle=oldangle-ang;
	    if(first) startloop=first=ajFalse;
	    else
	    {
		if(startloop)
		{
		    if(wheel)
		    {
			ajPolToRec(wradius-wheelgap,oldangle,&x1,&y1);
			ajPolToRec(wradius,angle,&x2,&y2);
			if(!text)
			    ajGraphLine(x1,y1,x2,y2);
			else
			    ajFmtPrintF(outf,"Line x1 %f y1 %f x2 %f y2 %f"
					" colour 0\n",x1,y1,x2,y2);
		    }
		    startloop=ajFalse;
		}
		else
		{
		    if(wheel)
		    {
			ajPolToRec(wradius,oldangle,&x1,&y1);
			ajPolToRec(wradius,angle,&x2,&y2);
			if(!text)
			    ajGraphLine(x1,y1,x2,y2);
			else
			    ajFmtPrintF(outf,"Line x1 %f y1 %f x2 %f y2 %f"
					" colour 0\n",x1,y1,x2,y2);
		    }
		}
	    }
	    pepwheel_plotresidue(*(ajStrStr(substr)+lc),radius+resgap,angle,
				 ajStrStr(squares),ajStrStr(octags),
				 ajStrStr(diamonds),text,outf,
				 xmin,xmax,ymin,ymax);
	    ++lc;
	    if(lc==len)
		break;
	}
	radius += nresgap;

    }

    if(!text)
	ajGraphCloseWin();
    else
	ajFileClose(&outf);

    ajStrDel(&strand);




    ajSeqDel(&seq);
    ajStrDel(&substr);

    ajExit();
    return 0;
}

/* @funcstatic pepwheel_drawocta **********************************************
**
** Draw an octagon
**
** @param [r] x [float] xpos
** @param [r] y [float] xpos
** @param [r] size [float] size
** @param [r] text [AjBool] text or graphic
** @param [w] outf [AjPFile] outfile
** @@
******************************************************************************/


static void pepwheel_drawocta(float x, float y, float size, AjBool text,
			      AjPFile outf)
{
    static float polyx[]=
    {
	-0.05, 0.05, 0.1, 0.1, 0.05, -0.05, -0.1, -0.1, -0.05
    }
    ;
    static float polyy[]=
    {
	0.1, 0.1, 0.05, -0.05, -0.1, -0.1, -0.05, 0.05, 0.1
    }
    ;
    ajint i;


    for(i=0;i<8;++i)
    {
	if(!text)
	    ajGraphLine(x+polyx[i]*size,y+polyy[i]*size,x+polyx[i+1]*size,
			y+polyy[i+1]*size);
	else
	    ajFmtPrintF(outf,"Line x1 %f y1 %f x2 %f y2 %f colour 0\n",
			x+polyx[i]*size,y+polyy[i]*size,x+polyx[i+1]*size,
			y+polyy[i+1]*size);
    }

    return;
}




/* @funcstatic pepwheel_plotresidue *******************************************
**
** Plot a residue
**
** @param [r] c [char] char to plot
** @param [r] r [float] radius
** @param [r] a [float] angle
** @param [r] squares [char*] residues for squares
** @param [r] octags [char*] residues for octagons
** @param [r] diamonds [char*] residues for diamonds
** @param [r] text [AjBool] text or graphic output
** @param [w] outf [AjPFile] outfile
** @param [r] xmin [float] co-ord
** @param [r] xmax [float] co-ord
** @param [r] ymin [float] co-ord
** @param [r] ymax [float] co-ord
** @@
******************************************************************************/


static void pepwheel_plotresidue(char c, float r, float a, char *squares,
				 char *octags, char *diamonds, AjBool text,
				 AjPFile outf, float xmin, float xmax,
				 float ymin, float ymax)
{
    float  x;
    float  y;

    static char cs[2];

    cs[1]='\0';
    *cs=c;

    ajPolToRec(r, a, &x, &y);

    if(x<xmin+.1 || x>xmax-.1 || y<ymin+.2 || y>ymax-.2)
	return;


    if(!text)
	ajGraphSetFore(AJB_PURPLE);

    if(strstr(squares,cs))
    {
	if(!text)
	{
	    ajGraphSetFore(AJB_BLUE);
	    ajGraphBox(x-0.025,y-0.022,0.05);
	}
	else
	    ajFmtPrintF(outf,"Rectangle x1 %f y1 %f x2 %f y2 %f colour %d\n",
			x-0.025,y-0.022,x-0.025+.05,y-0.022+.05,AJB_BLUE);
    }
    if(strstr(octags,cs))
    {
	if(!text)
	    ajGraphSetFore(AJB_BLACK);
	pepwheel_drawocta(x,y+0.003,0.28,text,outf);
    }
    if(strstr(diamonds,cs))
    {
	if(!text)
	{
	    ajGraphSetFore(AJB_RED);
	    ajGraphDia(x-0.042,y-0.04,0.085);
	}
	else
	    ajFmtPrintF(outf,"Shaded Rectangle x1 %f y1 %f x2 %f y2 %f "
			"colour %d\n",
			x-0.025,y-0.022,x-0.025+.05,y-0.022+.05,AJB_RED);
    }

    if(!text)
    {
	ajGraphText(x,y,cs,0.5);
	ajGraphSetFore(AJB_BLACK);
    }
    else
	ajFmtPrintF(outf,"Text1 x1 %f y1 %f colour 0 size 0.5 %s\n",x,y,cs);

    return;
}

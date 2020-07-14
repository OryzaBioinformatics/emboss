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




static void pepwheel_plotresidue(char c, float r, float a,const  char *squares,
				 const char *circles, const char *diamonds,
				 float xmin,float xmax,
				 float ymin,float ymax);
static void pepwheel_drawocta(float x, float y, float size);




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
    AjPSeq seq      = NULL;
    AjPStr strand   = NULL;
    AjPStr substr   = NULL;
    AjPStr squares  = NULL;
    AjPStr diamonds = NULL;
    AjPStr octags   = NULL;
    AjBool wheel;
    AjBool amphipathic;
    AjPStr txt = NULL;
    AjPGraph  graph=0;
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

    float xmin = -1.0;
    float xmax =  1.0;
    float ymin = -0.75;
    float ymax =  0.75;

    float minresplot = 36.0;
    float resgap =     0.0533;
    float wheelgap =   0.00;
    float nresgap =    0.08;




    float angle;
    float oldangle;
    float ang;
    float radius;
    float wradius;
    float x1;
    float x2;
    float y1;
    float y2;


    ajGraphInit("pepwheel", argc, argv);


    seq         = ajAcdGetSeq("sequence");
    steps       = ajAcdGetInt("steps");
    turns       = ajAcdGetInt("turns");
    graph       = ajAcdGetGraph("graph");
    octags      = ajAcdGetString("octags");
    squares     = ajAcdGetString("squares");
    diamonds    = ajAcdGetString("diamonds");
    wheel       = ajAcdGetBool("wheel");
    amphipathic = ajAcdGetToggle("amphipathic");

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


    begin = ajSeqBegin(seq);
    end   = ajSeqEnd(seq);
    ajDebug("begin: %d end: %d\n", begin, end);
    strand = ajSeqStrCopy(seq);

    ajStrToUpper(&strand);
    ajStrAssSubC(&substr,ajStrStr(strand),begin-1,end-1);
    len = ajStrLen(substr);

    ajFmtPrintS(&txt,"PEPWHEEL of %s from %d to %d",ajSeqName(seq),
		begin,end);

    ajGraphOpenWin(graph,xmin,xmax,ymin,ymax);

    ajGraphSetFore(AJB_BLACK);
    ajGraphText(0.0,0.64,ajStrStr(txt),0.5);

    ajGraphSetFore(AJB_BLACK);

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
	    oldangle = angle;
	    angle    = oldangle-ang;
	    if(first)
		startloop = first = ajFalse;
	    else
	    {
		if(startloop)
		{
		    if(wheel)
		    {
			ajPolToRec(wradius-wheelgap,oldangle,&x1,&y1);
			ajPolToRec(wradius,angle,&x2,&y2);
			ajGraphLine(x1,y1,x2,y2);
		    }
		    startloop=ajFalse;
		}
		else
		    if(wheel)
		    {
			ajPolToRec(wradius,oldangle,&x1,&y1);
			ajPolToRec(wradius,angle,&x2,&y2);
			ajGraphLine(x1,y1,x2,y2);
		    }
	    }
	    pepwheel_plotresidue(*(ajStrStr(substr)+lc),radius+resgap,angle,
				 ajStrStr(squares),ajStrStr(octags),
				 ajStrStr(diamonds),
				 xmin,xmax,ymin,ymax);
	    ++lc;
	    if(lc==len)
		break;
	}
	radius += nresgap;

    }

    ajGraphCloseWin();

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
** @@
******************************************************************************/

static void pepwheel_drawocta(float x, float y, float size)
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
	ajGraphLine(x+polyx[i]*size,y+polyy[i]*size,x+polyx[i+1]*size,
			y+polyy[i+1]*size);

    return;
}




/* @funcstatic pepwheel_plotresidue *******************************************
**
** Plot a residue
**
** @param [r] c [char] char to plot
** @param [r] r [float] radius
** @param [r] a [float] angle
** @param [r] squares [const char*] residues for squares
** @param [r] octags [const char*] residues for octagons
** @param [r] diamonds [const char*] residues for diamonds
** @param [r] xmin [float] co-ord
** @param [r] xmax [float] co-ord
** @param [r] ymin [float] co-ord
** @param [r] ymax [float] co-ord
** @@
******************************************************************************/

static void pepwheel_plotresidue(char c, float r, float a, const char *squares,
				 const char *octags, const char *diamonds,
				 float xmin, float xmax,
				 float ymin, float ymax)
{
    float  x;
    float  y;

    static char cs[2];

    cs[1] = '\0';
    *cs   = c;

    ajPolToRec(r, a, &x, &y);

    if(x<xmin+.1 || x>xmax-.1 || y<ymin+.2 || y>ymax-.2)
	return;


    ajGraphSetFore(AJB_PURPLE);

    if(strstr(squares,cs))
    {
	ajGraphSetFore(AJB_BLUE);
	ajGraphBox(x-0.025,y-0.022,0.05);
    }

    if(strstr(octags,cs))
    {
	ajGraphSetFore(AJB_BLACK);
	pepwheel_drawocta(x,y+0.003,0.28);
    }

    if(strstr(diamonds,cs))
    {
	ajGraphSetFore(AJB_RED);
	ajGraphDia(x-0.042,y-0.04,0.085);
    }

    ajGraphText(x,y,cs,0.5);
    ajGraphSetFore(AJB_BLACK);
 
    return;
}

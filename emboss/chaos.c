/* @source chaos application
** @author Ian Longden
** @@
** Chaos produces a chaos plot.
** The original application is part of the ACEDB genome database
** package, written by ** Richard Durbin (MRC LMB, UK)
** rd@mrc-lmba.cam.ac.uk, and Jean Thierry-Mieg (CRBM du CNRS,
** France) mieg@crbm1.cnusc.fr
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
#include "ajax.h"




/* @prog chaos ****************************************************************
**
** Create a chaos game representation plot for a sequence
**
******************************************************************************/

int main(int argc, char **argv)
{
    AjPSeq sequence = NULL;
    AjPGraph graph  = NULL;
    float x = 0.5;
    float y = 0.5;
    float x2;
    const char *ptr;
    ajint i;
    ajint a = 0;
    ajint c = 0;
    ajint t = 0;
    ajint g = 0;
    ajint n = 0;
    char line[40];
    AjPStr  str=NULL;
    ajint   begin;
    ajint   end;
    ajint   len;

    ajGraphInit("chaos", argc, argv);

    sequence = ajAcdGetSeq("sequence");
    graph = ajAcdGetGraph("graph");

    ajGraphOpenWin(graph,-0.1,1.4,-0.1,1.1);

    str = ajStrNew();

    begin = ajSeqBegin(sequence) - 1;
    end   = ajSeqEnd(sequence) - 1;

    ajStrAssSub(&str,ajSeqStr(sequence),begin,end);


    ajStrToLower(&str);
    ptr = ajStrStr(str);

    len = end-begin+1;

    for(i=0;i<len;i++)
    {
	if(*ptr=='a')
	{
	    x *= 0.5;
	    y *= 0.5;
	    a++;
	}
	else if(*ptr=='g')
	{
	    x = (x+ 1.0)*0.5;
	    y = (y+ 1.0)*0.5;
	    g++;
	}
	else if(*ptr=='c')
	{
	    x *= 0.5;
	    y = (y+1.0 )*0.5;
	    c++;
	}
	else if(*ptr=='t')
	{
	    x = (x+1.0 )*0.5;
	    y *= 0.5;
	    t++;
	}
	else
	{
	    x = 0.5;
	    y = 0.5;
	    n++;
	}
	x2 = x + 0.0001;
	ajGraphLines(&x,&y,&x2,&y,1);
	ptr++;
    }

    ajGraphTextEnd(0.0,0.0,"A");
    ajGraphTextEnd(0.0,1.0,"C");
    ajGraphTextStart(1.0,0.0,"T");
    ajGraphTextStart(1.0,1.0,"G");


    ajGraphSetCharSize(0.5);
    sprintf(line,"A %d",a);
    ajGraphTextStart(1.1,0.75,line);
    sprintf(line,"C %d",c);
    ajGraphTextStart(1.1,0.70,line);
    sprintf(line,"T %d",t);
    ajGraphTextStart(1.1,0.65,line);
    sprintf(line,"G %d",t);
    ajGraphTextStart(1.1,0.60,line);
    sprintf(line,"N %d",n);
    ajGraphTextStart(1.1,0.55,line);


    sprintf(line,"%cA %3.2f",'%',((float)a/(float)len)*
	    100.0);
    ajGraphTextStart(1.1,0.45,line);
    sprintf(line,"%cC %3.2f",'%',((float)c/(float)len)*
	    100.0);
    ajGraphTextStart(1.1,0.40,line);
    sprintf(line,"%cT %3.2f",'%',((float)t/(float)len)*
	    100.0);
    ajGraphTextStart(1.1,0.35,line);
    sprintf(line,"%cG %3.2f",'%',((float)g/(float)len)*
	    100.0);
    ajGraphTextStart(1.1,0.30,line);
    sprintf(line,"%cN %3.2f",'%',((float)n/(float)len)*
	    100.0);
    ajGraphTextStart(1.1,0.25,line);


    ajGraphCloseWin();
    ajStrDel(&str);

    ajExit();

    return 0;
}

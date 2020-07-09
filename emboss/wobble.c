/* @source wobble application
**
** Plot wobble base percentage
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
******************************************************************************/

#include "emboss.h"
#include <string.h>
#include <stdlib.h>


void checkstring(AjPStr *str);
void calcpc(char *seq, char *rev, int n, float **x, float **y, int *count,
	    int beg, char *gc, int window);
float get_mean(char *bases, char *s);


int main(int argc, char **argv)
{
    AjPFile    outf;
    AjPSeq     seq;
    AjPStr     gc;
    AjPStr     fwd;
    AjPStr     rev;
    
    
    AjPGraph   graph;
    AjPGraphData data;
    
    float      *x[6];
    float      *y[6];
    float      xt;
    float      mean;
    float      ymin;
    float      ymax;
    
    int        count[6];
    
    int        window;


    int        beg;
    int	       end;
    
    int        i;
    int        j;
    
    char *ftit[6]=
    {
	"F1","F2","F3","R1","R2","R3"
    };
    
    
    
	
    
    (void) ajGraphInit("wobble", argc, argv);

    seq       = ajAcdGetSeq("sequence");
    graph     = ajAcdGetGraphxy("graph");
    gc        = ajAcdGetString("bases");
    window    = ajAcdGetInt("window");
    outf      = ajAcdGetOutfile("outf");
    
    ajSeqToUpper(seq);
    ajStrToUpper(&gc);
    
    beg = ajSeqBegin(seq);
    end = ajSeqEnd(seq);

    fwd = ajStrNew();
    rev = ajStrNew();

    (void) ajStrAssSubC(&fwd,ajSeqChar(seq),beg-1,end-1);
    rev = ajStrNewC(ajStrStr(fwd));
    ajSeqReverseStr(&rev);


    checkstring(&gc);
    mean = get_mean(ajStrStr(gc),ajStrStr(fwd));
    ajFmtPrintF(outf,"Expected %s content in third position = %.2f\n",
		ajStrStr(gc),mean);
    
    for(i=0;i<6;++i)
    {
	calcpc(ajStrStr(fwd),ajStrStr(rev),i,x,y,count,beg,ajStrStr(gc),
	       window);

	data = ajGraphxyDataNewI(count[i]);

	ajGraphxySetOverLap(graph,ajFalse);

	for(j=0;j<count[i];++j)
	{
	    xt = x[i][j];
	    if(!xt)
	    {
		x[i][j]=x[i][j-1];
		y[i][j]=y[i][j-1];
	    }
	    data->x[j]=x[i][j];
	    data->y[j]=y[i][j];
	}

	ajGraphDataxyMaxMin(data->y,count[i],&ymin,&ymax);
	ajGraphDataxySetMaxima(data,(float)beg,(float)end,ymin,ymax);
	
	ajGraphDataxySetTypeC(data,"2D Plot");
	ajGraphxyAddGraph(graph,data);

	ajGraphxySetYTick(graph, ajTrue);
	
	ajGraphxyDataSetYtitleC(data,ajStrStr(gc));
	ajGraphxyDataSetXtitleC(data,"Sequence");
	ajGraphxyDataSetTitleC(data,ftit[i]);
	ajGraphDataObjAddLine(data,(float)beg,mean,(float)end,mean,4);
    }
    

    ajGraphxySetTitleDo(graph, ajTrue);
    ajGraphxySetMaxMin(graph,(float)beg,(float)end,0.0,100.0);
    
    ajGraphxySetYStart(graph,0.0);
    ajGraphxySetYEnd(graph,100.0);
    ajGraphxyTitleC(graph,"Wobble bases");

    ajGraphSetCharSize(0.7);
    
    ajGraphxyDisplay(graph,ajTrue);
    
 
    ajStrDel(&gc);
    ajStrDel(&fwd);
    ajStrDel(&rev);
    ajExit();
    return 0;
}



void calcpc(char *seq, char *rev, int n, float **x, float **y, int *count,
	    int beg, char *gc, int window)
{

    int len;
    int limit;
    
    int i;
    int j;
    
    int po;
    char *p;
    float sum;
    
    int nb;
    int cds;
    int z;
    
    limit = window*3;
    
    len = strlen(seq);
    if(n<3)
    {
	p = seq;
	po = n%3;
    }
    else
    {
	p=rev;
	po=len%3;
	po-=n%3;
	if(po<0)
	    po+=3;
    }
    nb = len-po;
    cds = nb/3 - window + 1;

    if(cds<1)
	ajFatal("Sequence too short for window");

    if(!n)
	for(i=0;i<6;++i)
	{
	    AJCNEW(x[i], cds);
	    AJCNEW(y[i], cds);
	    count[i]=cds;
	}
    

    for(i=0;i<cds;++i)
    {
	sum = (float)0.0;
	for(j=0;j<limit;j+=3)
	{
	    z = (i*3)+j+po+2;
	    if(z<len)
		if(strchr(gc,(int)p[z]))
		    ++sum;
	}
	
	x[n][i] = (float) ((i*3)+beg+(limit/2));
	y[n][i] = (float)((sum/(float)window)*(float)100.0);
    }

    return;
}

	    


void checkstring(AjPStr *str)
{
    AjPStr tmp;
    static char *bases="GCAT";
    int i;
    
    tmp = ajStrNewC("");
    for(i=0;i<4;++i)
	if(strchr(ajStrStr(*str),(int)bases[i]))
	    ajStrAppK(&tmp,bases[i]);
    (void) ajStrAssC(str,ajStrStr(tmp));
    
    if(ajStrLen(tmp) >3)
	ajFatal("Specifying ACG&T is meaningless");

    if(ajStrLen(tmp)<1)
	ajFatal("No bases specified");

    ajStrDel(&tmp);
    return;
}



float get_mean(char *bases, char *s)
{
    int na;
    int nc;
    int ng;
    int nt;
    float tot;
    float sum;
    char  *p;
    int   c;

    na=nc=ng=nt=0;
    ajCodComp(&na,&nc,&ng,&nt,s);
    sum = (float) 0.0;
    p=bases;
    
    while((c=*p))
    {
	if(c=='A') sum+=na;
	else if(c=='C') sum+=nc;
	else if(c=='G') sum+=ng;
	else if(c=='T') sum+=nt;
	++p;
    }

    tot = (float) na+nc+ng+nt;
    
    return (sum/tot) * (float) 100.0;
}

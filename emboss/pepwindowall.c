/* @source pepwindowall application
**
** Displays protein hydropathy.
** @author: Copyright (C) Ian Longden (il@sanger.ac.uk)
** @@
** Original program by Jack Kyte and Russell F. Doolittle.
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

#include "limits.h"
#include <float.h>
#include "emboss.h"

#define AZ 28




static AjBool pepwindowall_getnakaidata(AjPFile file, float matrix[]);




/* @prog pepwindowall *********************************************************
**
** Displays protein hydropathy of a set of sequences
**
******************************************************************************/

int main(int argc, char **argv)
{
    AjPFile datafile;
    AjPStr aa0str = 0;
    AjPSeqset seqset;
    AjPGraphPlpData graphdata;
    AjPGraph mult;
    const char *seq;
    const char *s1;
    ajint *position;
    ajint i;
    ajint j;
    ajint k;
    ajint w;
    ajint a;
    ajint midpoint,llen,maxlen;
    float total;
    float matrix[AZ];
    float min = 555.5;
    float max = -555.5;
    float v   = 0.;
    float v1  = 0.;
    float ymin = 64000.;
    float ymax = -64000.;
    ajint beg;
    ajint end;

    ajGraphInit("pepwindowall", argc, argv);

    seqset   = ajAcdGetSeqset("sequences");
    mult     = ajAcdGetGraphxy("graph");
    datafile = ajAcdGetDatafile("datafile");
    llen     = ajAcdGetInt("length");

    if(!pepwindowall_getnakaidata(datafile,&matrix[0]))
	ajExitBad();

    maxlen   = ajSeqsetLen(seqset);
    aa0str   = ajStrNewL(maxlen);
    midpoint = (ajint)((llen+1)/2);

    AJCNEW(position, ajSeqsetLen(seqset));

    for(i=0;i<ajSeqsetSize(seqset);i++)
    {
	seq = ajSeqsetSeq(seqset, i);
	ajStrClear(&aa0str);

	graphdata = ajGraphPlpDataNewI(ajSeqsetLen(seqset));
	ajGraphPlpDataSetTypeC(graphdata,"Overlay 2D Plot");
	ymin = 64000.;
	ymax = -64000.;


	for(k=0;k<ajSeqsetLen(seqset) ;k++)
	    graphdata->x[k] = FLT_MIN;

	s1 = seq;
	k = 0;
	w = 0;
	while(*s1 != '\0')
	{
	    if(ajAZToInt(*s1) != 27 )
	    {
		ajStrAppK(&aa0str,(char)ajAZToInt(*s1));
		position[k++]= w+midpoint;
	    }
	    w++;
	    s1++;
	}



	s1 = ajStrStr(aa0str);
	for(j=0;j<k-llen;j++)
	{
	    total = 0;
	    for(w=0;w<llen;w++)
		total = total + matrix[(ajint)s1[w]];

	    total = total/(float)llen;
	    v1 = (float)position[j];
	    graphdata->x[position[j]] = v1;
	    v    = graphdata->y[position[j]] = total;
	    ymin = (ymin<v) ? ymin : v;
	    ymax = (ymax>v) ? ymax : v;

	    if(total > max)
		max = total;
	    if(total < min)
		min = total;
	    s1++;
	}


	beg = 0;
	while(graphdata->x[beg]<0.00001)
	    ++beg;
	graphdata->numofpoints -= beg;

	for(a=0;a<graphdata->numofpoints;++a)
	{
	    graphdata->x[a] = graphdata->x[a+beg];
	    graphdata->y[a] = graphdata->y[a+beg];
	}
	end = graphdata->numofpoints-1;

	while(graphdata->x[end--]<0.00001)
	    --graphdata->numofpoints;

	end = ajSeqsetLen(seqset)-1;
	while(!graphdata->x[end])
	    --end;

	ajGraphPlpDataSetMaxima(graphdata,(float)graphdata->x[0],
			       (float)graphdata->x[graphdata->numofpoints-1],
			       ymin,ymax);

	ajGraphPlpDataSetYTitleC(graphdata,"Hydropathy");
	ajGraphPlpDataSetXTitleC(graphdata,"Sequence");

	ajGraphDataAdd(mult,graphdata);
    }

    min = min*1.1;
    max = max*1.1;

    ajGraphxySetGaps(mult,AJTRUE);
    ajGraphxySetOverLap(mult,AJTRUE);

    ajGraphxySetMaxMin(mult,0.0,(float)ajSeqsetLen(seqset),min,max);
    ajGraphSetTitleC(mult,"Pepwindowall");


    ajGraphxyDisplay(mult,AJTRUE);

    ajExit();

    return 0;
}




static AjBool pepwindowall_getnakaidata(AjPFile file, float matrix[])
{
    AjPStr buffer = NULL;
    AjPStr buf2   = NULL;
    AjPStr delim  = NULL;
    AjBool description = ajFalse;
    AjPStrTok token;
    ajint line = 0;
    const char *ptr;
    ajint cols;


    if(!file)
	return 0;


    delim  = ajStrNewC(" :\t\n");
    buffer = ajStrNew();
    buf2   = ajStrNew();


    while(ajFileGets(file,&buffer))
    {
	ptr = ajStrStr(buffer);
	if(*ptr == 'D')			/* description */
	    description = ajTrue;
	else if(*ptr == 'I')
	    line = 1;
	else if(line == 1)
	{
	    line++;
            ajStrClean(&buffer);
	    token = ajStrTokenInit(buffer,ajStrStr(delim));
	    cols = ajStrTokenCount(buffer,ajStrStr(delim));
	    ajDebug("num of cols = %d\n",cols);

	    ajStrToken(&buf2,&token,ajStrStr(delim));
	    ajStrToFloat(buf2,&matrix[0]);

	    ajStrToken(&buf2,&token,ajStrStr(delim));
	    ajStrToFloat(buf2,&matrix[17]);

	    ajStrToken(&buf2,&token,ajStrStr(delim));
	    ajStrToFloat(buf2,&matrix[13]);

	    ajStrToken(&buf2,&token,ajStrStr(delim));
	    ajStrToFloat(buf2,&matrix[3]);

	    ajStrToken(&buf2,&token,ajStrStr(delim));
	    ajStrToFloat(buf2,&matrix[2]);

	    ajStrToken(&buf2,&token,ajStrStr(delim));
	    ajStrToFloat(buf2,&matrix[16]);

	    ajStrToken(&buf2,&token,ajStrStr(delim));
	    ajStrToFloat(buf2,&matrix[4]);

	    ajStrToken(&buf2,&token,ajStrStr(delim));
	    ajStrToFloat(buf2,&matrix[6]);

	    ajStrToken(&buf2,&token,ajStrStr(delim));
	    ajStrToFloat(buf2,&matrix[7]);

	    ajStrToken(&buf2,&token,ajStrStr(delim));
	    ajStrToFloat(buf2,&matrix[8]);

	    ajStrTokenClear(&token);
	}
	else if(line == 2)
	{
	    line++;
	    ajStrClean(&buffer);
	    token = ajStrTokenInit(buffer,ajStrStr(delim));
	    cols  = ajStrTokenCount(buffer,ajStrStr(delim));
	    ajStrToken(&buf2,&token,ajStrStr(delim));
	    ajStrToFloat(buf2,&matrix[11]);

	    ajStrToken(&buf2,&token,ajStrStr(delim));
	    ajStrToFloat(buf2,&matrix[10]);

	    ajStrToken(&buf2,&token,ajStrStr(delim));
	    ajStrToFloat(buf2,&matrix[12]);

	    ajStrToken(&buf2,&token,ajStrStr(delim));
	    ajStrToFloat(buf2,&matrix[5]);

	    ajStrToken(&buf2,&token,ajStrStr(delim));
	    ajStrToFloat(buf2,&matrix[15]);

	    ajStrToken(&buf2,&token,ajStrStr(delim));
	    ajStrToFloat(buf2,&matrix[18]);

	    ajStrToken(&buf2,&token,ajStrStr(delim));
	    ajStrToFloat(buf2,&matrix[19]);

	    ajStrToken(&buf2,&token,ajStrStr(delim));
	    ajStrToFloat(buf2,&matrix[22]);

	    ajStrToken(&buf2,&token,ajStrStr(delim));
	    ajStrToFloat(buf2,&matrix[24]);

	    ajStrToken(&buf2,&token,ajStrStr(delim));
	    ajStrToFloat(buf2,&matrix[21]);

	    ajStrTokenClear(&token);
	}
    }


    ajStrDel(&buffer);
    ajStrDel(&buf2);
    ajStrDel(&delim);

    return description;
}

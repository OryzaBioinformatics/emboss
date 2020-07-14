/* @source pepwindow application
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

#include "emboss.h"

#define AZ 28




static AjBool pepwindow_getnakaidata(AjPFile file, float matrix[]);




/* @prog pepwindow ************************************************************
**
** Displays protein hydropathy
**
******************************************************************************/

int main(int argc, char **argv)
{
    AjPFile datafile;
    AjPStr aa0str = 0;
    char *s1;
    AjPSeq seq;
    ajint llen;
    float matrix[AZ];
    ajint i;
    ajint midpoint;
    ajint j;
    AjPGraphData graphdata;
    AjPGraph mult;
    float min = 555.5;
    float max = -555.5;
    float total;

    ajGraphInit("pepwindow", argc, argv);

    seq = ajAcdGetSeq("sequence");

    mult     = ajAcdGetGraphxy("graph");
    datafile = ajAcdGetDatafile("datafile");
    llen     = ajAcdGetInt("length");

    s1 = ajStrStr(ajSeqStr(seq));

    aa0str = ajStrNewL(ajSeqLen(seq)+1);

    graphdata = ajGraphxyDataNewI(ajSeqLen(seq)-llen);

    midpoint = (ajint)((llen+1)/2);

    ajGraphDataxySetTypeC(graphdata,"2D Plot");

    ajGraphxyAddGraph(mult,graphdata);

    for(i=0;i<ajSeqLen(seq);i++)
	ajStrAppK(&aa0str,(char)ajAZToInt(*s1++));


    if(!pepwindow_getnakaidata(datafile,&matrix[0]))
	exit(-1);

    s1 = ajStrStr(aa0str);

    for(i=0;i<ajSeqLen(seq)-llen;i++)
    {
	total = 0;
	for(j=0;j<llen;j++)
	    total += matrix[(ajint)s1[j]];


	total /= (float)llen;
	graphdata->x[i] = (float)i+midpoint;
	graphdata->y[i] = total;
	if(total > max)
	    max= total;
	if(total < min)
	    min = total;

	s1++;
    }

    ajGraphDataxySetMaxima(graphdata,0.,(float)ajSeqLen(seq),min,max);

    min = min*1.1;
    max = max*1.1;

    ajGraphDataxySetMaxMin(graphdata,0.0,(float)ajSeqLen(seq),min,max);
    ajGraphxySetMaxMin(mult,0.0,(float)ajSeqLen(seq),min,max);

    ajGraphxyDisplay(mult,AJTRUE);

    ajExit();

    return 0;
}




/* @funcstatic pepwindow_getnakaidata *****************************************
**
** Read the NAKAI (AAINDEX) data file
**
** @param [r] file [AjPFile] Input file
** @param [w] matrix [float[]] Data values for each amino acid
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/


static AjBool pepwindow_getnakaidata(AjPFile file, float matrix[])
{
    AjPStr buffer = NULL;
    AjPStr buf2   = NULL;
    AjPStr delim  = NULL;
    AjPStr description = NULL;
    AjPStrTok token;
    ajint line = 0;
    char *ptr;


    if(!file)
	return 0;

    delim  = ajStrNewC(" :\t\n");
    buffer = ajStrNew();
    buf2   = ajStrNew();
    description = ajStrNew();



    while(ajFileGets(file,&buffer))
    {
	ptr = ajStrStr(buffer);
	if(*ptr == 'D')			/* save description */
	    ajStrAssS(&description, buffer);
	else if(*ptr == 'I')
	    line = 1;
	else if(line == 1)
	{
	    line++;
	    ajStrClean(&buffer);

	    token = ajStrTokenInit(buffer,ajStrStr(delim));

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
    ajFileClose(&file);

    ajStrDel(&buffer);
    ajStrDel(&description);
    ajStrDel(&buf2);
    ajStrDel(&delim);

    return ajTrue;
}

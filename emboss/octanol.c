/* @source octanol application
**
** Displays protein hydropathy.
** @author Copyright (C) Ian Longden (il@sanger.ac.uk)
** @@
** From White and Wimley Annu Rev. Biophys. Biomol. Struct. 1999. 28:319-65
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




static ajint octanol_getwhitewimbleydata(AjPFile file, float matrix[],
					 float err[], float matrix2[],
					 float err2[]);




/* @prog octanol **************************************************************
**
** Displays protein hydropathy
**
******************************************************************************/

int main(int argc, char **argv)
{
    AjPFile datafile;
    float matrix[AZ];
    float err[AZ];		/* interface values */
    float matrix2[AZ];
    float err2[AZ];		/* octanol values   */
    AjPSeq seq;
    ajuint llen;
    ajuint i;
    ajuint j;
    AjPGraphPlpData graphdata;
    AjPGraphPlpData graphdata2;
    AjPGraphPlpData graphdata3;
    AjPGraph mult;
    float min = 555.5;
    float max = -555.5;
    float total  = 0;
    float total2 = 0;
    const char *s1;
    float midpoint;
    AjBool interface;
    AjBool octanol;
    AjBool difference;
    float xmin1 = 64000.;
    float xmax1 = -64000.;
    float xmin2 = 64000.;
    float xmax2 = -64000.;
    float xmin3 = 64000.;
    float xmax3 = -64000.;
    float ymin1 = 64000.;
    float ymax1 = -64000.;
    float ymin2 = 64000.;
    float ymax2 = -64000.;
    float ymin3 = 64000.;
    float ymax3 = -64000.;
    float v;
    float flen;
    ajuint ilen;
    ajuint tui;
    
    ajGraphInit("octanol", argc, argv);

    seq = ajAcdGetSeq("sequence");
    ilen = ajSeqGetLen(seq);
    tui  = ajSeqGetLen(seq);
    flen = (float) tui;

    mult = ajAcdGetGraphxy("graph");
    datafile  = ajAcdGetDatafile("datafile");
    llen = ajAcdGetInt("width");
    interface  = ajAcdGetBool("interfaceplot");
    octanol = ajAcdGetBool("octanolplot");
    difference = ajAcdGetBool("differenceplot");

    if(!octanol_getwhitewimbleydata(datafile,&matrix[0],&err[0],&matrix2[0],
				    &err2[0]))
	ajFatal("Could not read data file");

    graphdata = ajGraphPlpDataNewI(ilen-llen);
    ajGraphPlpDataSetColour(graphdata,GREEN);

    if(interface)
    {
	ajGraphDataAdd(mult,graphdata);
	ajGraphPlpDataSetYTitleC(graphdata,"interface");
	ajGraphPlpDataSetTypeC(graphdata,"2D Plot");
    }


    graphdata3 = ajGraphPlpDataNewI(ilen-llen);
    ajGraphPlpDataSetColour(graphdata3,BLACK);

    if(difference)
    {
	ajGraphDataAdd(mult,graphdata3);
	ajGraphPlpDataSetYTitleC(graphdata3,"difference");
	ajGraphPlpDataSetTypeC(graphdata3,"2D Plot");
    }

    graphdata2 = ajGraphPlpDataNewI(ilen-llen);
    ajGraphPlpDataSetColour(graphdata2,RED);

    if(octanol)
    {
	ajGraphDataAdd(mult,graphdata2);
	ajGraphPlpDataSetYTitleC(graphdata2,"octanol");
	ajGraphPlpDataSetTypeC(graphdata,"Overlay 2D Plot");
	ajGraphPlpDataSetTypeC(graphdata2,"Overlay 2D Plot");
	ajGraphPlpDataSetTypeC(graphdata3,"Overlay 2D Plot");
    }

    ajGraphAddLine(mult,0.0,0.0,flen,0.0,BLACK);

    midpoint = ((float)llen+(float)1.0)/(float)2.0;

    ajSeqFmtLower(seq);

    s1 = ajStrGetPtr(ajSeqGetSeqS(seq));

    for(j=0;j<llen;j++)
    {
	total += matrix[ajAZToInt(s1[j])];
	total2 += matrix2[ajAZToInt(s1[j])];
    }

    for(i=0;i<ilen-llen;i++)
    {
	v = graphdata->x[i] = (float)i+midpoint;
	xmin1 = (xmin1<v) ? xmin1 : v;
	xmax1 = (xmax1>v) ? xmax1 : v;

	v = graphdata->y[i] = -total;
	ymin1 = (ymin1<v) ? ymin1 : v;
	ymax1 = (ymax1>v) ? ymax1 : v;

	v = graphdata2->x[i] = (float)i+midpoint;
	xmin2 = (xmin2<v) ? xmin2 : v;
	xmax2 = (xmax2>v) ? xmax2 : v;

	v = graphdata2->y[i] = -total2;
	ymin2 = (ymin2<v) ? ymin2 : v;
	ymax2 = (ymax2>v) ? ymax2 : v;

	v = graphdata3->x[i] = (float)i+midpoint;
	xmin3 = (xmin3<v) ? xmin3 : v;
	xmax3 = (xmax3>v) ? xmax3 : v;

	v = graphdata3->y[i] = -(total2-total);
	ymin3 = (ymin3<v) ? ymin3 : v;
	ymax3 = (ymax3>v) ? ymax3 : v;

	if(-total > max)
	    max= -total;
	if(-total < min)
	    min = -total;
	if(-total2 > max)
	    max= -total2;
	if(-total2 < min)
	    min = -total2;

	total -= matrix[ajAZToInt(s1[0])];
	total += matrix[ajAZToInt(s1[llen])];
	total2 -= matrix2[ajAZToInt(s1[0])];
	total2 += matrix2[ajAZToInt(s1[llen])];


	s1++;
    }
    min = min*(float)1.1;
    max = max*(float)1.1;

    ajGraphxySetMaxMin(mult,0.0,flen,min,max);
    ajGraphPlpDataSetMaxima(graphdata,xmin1,xmax1,ymin1,ymax1);
    ajGraphPlpDataSetMaxima(graphdata2,xmin2,xmax2,ymin2,ymax2);
    ajGraphPlpDataSetMaxima(graphdata3,xmin3,xmax3,ymin3,ymax3);

    ajGraphxyDisplay(mult,AJTRUE);

    ajFileClose(&datafile);
    ajSeqDel(&seq);
    ajGraphxyDel(&mult);

    if(!interface)
	ajGraphPlpDataDel(&graphdata);

    if(!octanol)
	ajGraphPlpDataDel(&graphdata2);

    if(!difference)
	ajGraphPlpDataDel(&graphdata3);

    embExit();

    return 0;
}




static ajint octanol_getwhitewimbleydata(AjPFile file, float matrix[],
					 float err[], float matrix2[],
					 float err2[])
{
    AjPStr buffer = NULL;
    AjPStr buf2   = NULL;
    AjPStr delim  = NULL;
    AjPStrTok token;
    const char *s1;
    ajint cols;
    ajint matpos;



    if(!file)
	return 0;

    delim  = ajStrNewC(" :\t\n");
    buffer = ajStrNew();
    buf2   = ajStrNew();



    while(ajFileGets(file,&buffer))
    {
	s1 = ajStrGetPtr(buffer);
	if(*s1 == '#')			/* ignore lines */
	    continue;
	if(ajStrGetLen(buffer) <= 1)	/* ignore empty lines */
	  continue;

	token = ajStrTokenNewS(buffer,delim);
	cols = ajStrParseCountS(buffer,delim);
	ajDebug("number of cols = %d\n",cols);

	ajStrTokenNextParseS(&token,delim,&buf2); /* get AA char code */
	ajStrFmtLower(&buf2);

	s1 = ajStrGetPtr(buf2);		/* convert code to integer */
	matpos = (ajint)s1[0];
	matpos -= 97;

	ajDebug("octanol buf2 %d '%c' matpos %d (%d)\n",
		s1[0], s1[0], matpos, ajStrGetLen(buffer));
	ajStrTokenNextParseS(&token,delim,&buf2); /* get interface value */
	ajStrToFloat(buf2,&matrix[matpos]);

	ajStrTokenNextParseS(&token,delim,&buf2); /* get +/- error value */
	ajStrToFloat(buf2,&err[matpos]);

	ajStrTokenNextParseS(&token,delim,&buf2); /* get octanol value value */
	ajStrToFloat(buf2,&matrix2[matpos]);

	ajStrTokenNextParseS(&token,delim,&buf2); /* get +/- error value */
	ajStrToFloat(buf2,&err2[matpos]);

	ajStrTokenDel(&token);
    }

    ajStrDel(&delim);
    ajStrDel(&buffer);
    ajStrDel(&buf2);


    return 1;
}

/* @source cirdna application
**
** Draws circular maps of DNA constructs
** @author: Copyright (C) Nicolas Tourasse (tourasse@biotek.uio.no),
** Biotechnology Centre of Oslo, Norway.
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
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#define MAXLABELS 100

void ReadInput(AjPFile infile, float *Start, float *End);
AjPStr ReadGroup(AjPFile infile, float *From, float *To, AjPStr *Name2,
		 char *FromSymbol, char *ToSymbol, AjPStr *Style2,
		 ajint *NumLabels, ajint *NumNames, ajint *Color);
float TextGroup(float TextHeight, float TextLength, AjPStr *Name2,
		ajint NumLabels, ajint *NumNames, AjPStr GroupName,
		AjPStr *Style2, float *From, float *To, float BlockHeight,
		AjPStr PosTicks);
float TextGroupStr(AjPStr *Name2, ajint NumLabels, ajint *NumNames,
		   AjPStr GroupName, float TextCoef, AjPStr *Style2,
		   float *From, float *To, float BlockHeight, AjPStr PosTicks);
float HeightGroup(float posblock, float posrange, float postext,
		  float TickHeight, float BlockHeight, float RangeHeight,
		  AjPStr *Name2, AjPStr *Style2, ajint NumLabels,
		  ajint *NumNames, AjPStr PosTicks, AjPStr PosBlocks,
		  ajint Adjust);
ajint OverlapTextGroup(AjPStr *Name2, AjPStr *Style2, ajint NumLabels,
		     float *From, float *To, float Start, float End,
		     AjPStr PosTicks, ajint *Adjust);
AjBool OverlapTickRuler(ajint NumGroups, ajint *NumLabels, float *From,
			AjPStr PosTicks, ajint RulerTick);
void DrawGroup(float xDraw, float yDraw, float posblock, float posrange,
	       float postext, float TickHeight, float BlockHeight,
	       float RangeHeight, float RealLength, float TextLength,
	       float TextHeight, float Radius, float RadiusMax, float *From,
	       float *To, AjPStr *Name2, char *FromSymbol, char *ToSymbol,
	       AjPStr *Style2, AjPStr InterSymbol, AjPStr InterTicks,
	       ajint NumLabels, AjPStr GroupName, float OriginAngle,
	       ajint *NumNames, AjPStr PosTicks, AjPStr PosBlocks, ajint *Adjust,
	       ajint InterColor, ajint *Color);
float TextRuler(float Start, float End, ajint GapSize, float TextLength,
		float TextHeight, AjPStr PosTicks, ajint NumGroups,
		ajint *NumLabels);
float TextRulerStr(float Start, float End, ajint GapSize, float TextCoef,
		   AjPStr PosTicks, ajint NumGroups, ajint *NumLabels);
float HeightRuler(float Start, float End, ajint GapSize, float postext,
		  float TickHeight, AjPStr PosTicks, ajint NumGroups,
		  ajint *NumLabels);
void DrawRuler(float xDraw, float yDraw, float Start, float End,
	       float RealLength, float Radius, float TickHeight,
	       float OriginAngle, ajint GapSize, AjPStr TickLines,
	       float TextLength, float TextHeight, float postext,
	       ajint NumGroups, ajint *NumLabels, float *From, AjPStr PosTicks,
	       ajint Color);
void DrawTicks(float xDraw, float yDraw, float RealLength, float Radius,
	       float TickHeight, float From, AjPStr Name2, float OriginAngle,
	       float TextLength, float TextHeight, float postext,
	       AjPStr PosTicks, ajint NumNames, ajint Adjust, ajint Color);
void DrawBlocks(float xDraw, float yDraw, float RealLength, float Radius,
		ajint BlockHeight, float From, float To, AjPStr Name2,
		float postext, float OriginAngle, AjPStr PosBlocks,
		ajint NumNames, ajint Adjust, ajint Color);
void DrawRanges(float xDraw, float yDraw, float RealLength, float Radius,
		float RangeHeight, float From, float To, char FromSymbol,
		char ToSymbol, AjPStr Name2, float OriginAngle,
		ajint NumNames, float postext, ajint Adjust, ajint Color);
void InterBlocks(float xDraw, float yDraw, float RealLength, float Radius,
		 float BlockHeight, float From, float To, float OriginAngle,
		 AjPStr InterSymbol, ajint Color);
void DrawArrowHeadsOnCurve(float xDraw, float yDraw, float RealLength,
			   float Height, float Length, float Radius,
			   float Angle, float OriginAngle, ajint Way);
void DrawBracketsOnCurve(float xDraw, float yDraw, float RealLength,
			 float Height, float Length, float Radius,
			 float Angle, float OriginAngle, ajint Way);
void HorTextPile(float x, float y, float Radius, float StartAngle,
		 float EndAngle, AjPStr Name2, float postext, ajint NumNames);
float HorTextPileHeight(float postext, ajint NumNames);
float HorTextPileLengthMax(AjPStr Name2, ajint NumNames);
float ComputeAngle(float RealLength, float Length, float OriginAngle);
AjPStr Style[MAXLABELS][MAXLABELS],Name[MAXLABELS][MAXLABELS];

int main(int argc, char **argv)
{
    AjPGraph graph;
    ajint i;
    ajint j;
    ajint GapSize;
    ajint NumLabels[MAXLABELS];
    ajint NumNames[MAXLABELS][MAXLABELS];
    ajint NumGroups;
    ajint InterColor;
    ajint Color[MAXLABELS][MAXLABELS];
    ajint Adjust[MAXLABELS][MAXLABELS];
    ajint AdjustMax[MAXLABELS];
    char FromSymbol[MAXLABELS][MAXLABELS];
    char ToSymbol[MAXLABELS][MAXLABELS];
    float xDraw;
    float yDraw;
    float Radius;
    float RadiusMax;
    float DrawRadius;
    float OriginAngle;
    float From[MAXLABELS][MAXLABELS];
    float To[MAXLABELS][MAXLABELS];
    float TotalHeight;
    float GroupHeight[MAXLABELS];
    float RulerHeight;
    float Width;
    float Height;
    float Border;
    float Start;
    float End;
    float DrawLength;
    float RealLength;
    float TickHeight;
    float BlockHeight;
    float RangeHeight;
    float TextLength;
    float TextHeight;
    float GapGroup;
    float posblock;
    float posrange;
    float postext;
    AjPFile infile;
    AjPStr line;
    AjPStr GroupName[MAXLABELS];
    AjPStr InterSymbol;
    AjPStr InterTicks;
    AjPStr PosTicks;
    AjPStr TickLines;
    AjPStr PosBlocks;
    float charsize;
    float minsize;


    /* read the ACD file for graphical programs */
    ajGraphInit("cirdna", argc, argv);

    /* get the angle of the molecule's origin */
    OriginAngle = ajAcdGetFloat("originangle");

    /* get the position of the ticks */
    PosTicks = ajAcdGetString("posticks");

    /* get the position of the text for blocks */
    PosBlocks = ajAcdGetString("posblocks");

    /* to draw or not to draw junctions to link blocks */
    InterSymbol = ajAcdGetString("intersymbol");
    /* get the color of junctions used to link blocks */
    InterColor = ajAcdGetInt("intercolor");

    /* to draw or not to draw junctions between ticks */
    InterTicks = ajAcdGetString("interticks");

    /* get the size of the intervals between the ruler's ticks */
    GapSize = ajAcdGetInt("gapsize");
    /* to draw or not to draw vertical lines at ruler's ticks */
    TickLines = ajAcdGetString("ticklines");


    /* set the output graphical context */
    graph = ajAcdGetGraph("graphout");

    /* get the input file */
    infile = ajAcdGetInfile("inputfile");

    /* length and height of text */
    TextHeight = 20;
    TextLength = TextHeight+50;


    /* read the start and end positions */
    ReadInput(infile, &Start, &End);

    /* compute the real length of the molecule */
    RealLength = (End - Start) + 1;

    /* height of a tick, a block, and a range */
    TickHeight = 10*ajAcdGetFloat("tickheight");
    if( ajStrMatchCaseC(PosBlocks, "Out") ) BlockHeight = 10*ajAcdGetFloat("blockheight");
    else BlockHeight = TextHeight+10;
    RangeHeight = 10*ajAcdGetFloat("rangeheight");

    /* set the relative positions of elements of a group */
    posblock = 0;
    posrange = 0;
    GapGroup = 10*ajAcdGetFloat("gapgroup");

    /* open the window in which the graphics will be drawn */
    DrawLength = 800;
    Border = TickHeight+50+TextLength;
    Width = DrawLength + 2*Border;
    Width*=(640.0/480.0);		/* to get a circle, not an oval */
    Height = DrawLength + 2*Border;
    ajGraphOpenWin(graph, 0, Width, 0, Height);

    /* coordinates of the circle's center */
    xDraw = 1.0*Width/2;
    yDraw = 1.0*Height/2;
    /* radius of the outermost circle */
    Radius = RadiusMax = 1.0*DrawLength/2;


    /* read the contents of the groups */
    line = ajStrNew();
    ajFileSeek(infile, 0L, 0);
    i = 0;
    while( ajFileReadLine(infile, &line) )
    {
	if( ajStrPrefixC(line, "group") )
	{
	    GroupName[i] = ReadGroup(infile, From[i], To[i], Name[i],
				     FromSymbol[i], ToSymbol[i], Style[i],
				     &NumLabels[i], NumNames[i], Color[i]);
	    i++;
	}
    }
    NumGroups = i;

    /* remove the beginning of the molecule in case it doesn't begin at 1 */
    for(i=0; i<NumGroups; i++) for(j=0; j<NumLabels[i]; j++)
    {
	From[i][j]-=( Start-1 );
	To[i][j]-=( Start-1 );
    }

    /* compute the character size that fits all groups, including the ruler */
    minsize = 100.0;
    charsize = TextRuler(Start, End, GapSize, TextLength, TextHeight,
			 PosTicks, NumGroups, NumLabels);
    if( charsize<minsize ) minsize = charsize;
    for(i=0; i<NumGroups; i++)
    {
	charsize = TextGroup(TextHeight, TextLength, Name[i], NumLabels[i],
			     NumNames[i], GroupName[i], Style[i], From[i],
			     To[i], BlockHeight, PosTicks);
	if( charsize<minsize )
	    minsize = charsize;
    }
    ajGraphSetDefCharSize(minsize);


    /* find whether horizontal text strings overlap within a group */
    postext = (ajGraphTextHeight(0, 0, 1, 1)+3)*ajAcdGetFloat("postext");
    for(i=0; i<NumGroups; i++)
	AdjustMax[i] = OverlapTextGroup(Name[i], Style[i], NumLabels[i],
					From[i], To[i], Start, End, PosTicks,
					Adjust[i]);


    /* compute the height of the ruler */
    RulerHeight = HeightRuler(Start, End, GapSize, postext, TickHeight,
			      PosTicks, NumGroups, NumLabels);
    if(!RulerHeight)
	ajDebug("Err: Ruler height");

    /* compute the height of the groups */
    TotalHeight = 0.0;
    for(i=0; i<NumGroups; i++)
    {
	GroupHeight[i] = HeightGroup(posblock, posrange, postext,
				     TickHeight, BlockHeight, RangeHeight,
				     Name[i], Style[i], NumLabels[i],
				     NumNames[i], PosTicks, PosBlocks,
				     AdjustMax[i]);
	TotalHeight += (GroupHeight[i]+GapGroup);
    }

    /*
     *  decrease the radius such that the innermost group is not
     *  compressed in the centre of the circle
     */
    DrawRadius = Radius - (TotalHeight/NumGroups);

    /*
     *  if the groups are too big, resize them such that they fit in the
     *  window
     */
    if( TotalHeight<DrawRadius )
	TotalHeight = DrawRadius;
    TickHeight/=(TotalHeight/DrawRadius);
    BlockHeight/=(TotalHeight/DrawRadius);
    RangeHeight/=(TotalHeight/DrawRadius);
    TextHeight/=(TotalHeight/DrawRadius);
    TextLength/=(TotalHeight/DrawRadius);
    postext/=(TotalHeight/DrawRadius);
    posblock/=(TotalHeight/DrawRadius);
    posrange/=(TotalHeight/DrawRadius);
    GapGroup/=(TotalHeight/DrawRadius);

    /* 
     *  the groups having been resized, recompute the character size that
     * fits all groups, including the ruler
     */
    minsize = 100.0;
    charsize = TextRulerStr(Start, End, GapSize, (TotalHeight/DrawRadius),
			    PosTicks, NumGroups, NumLabels);
    if( charsize<minsize ) minsize = charsize;
    for(i=0; i<NumGroups; i++)
    {
	charsize = TextGroupStr(Name[i], NumLabels[i], NumNames[i],
				GroupName[i], (TotalHeight/DrawRadius),
				Style[i], From[i], To[i], BlockHeight,
				PosTicks);
	if( charsize<minsize )
	    minsize = charsize;
    }
    ajGraphSetDefCharSize(minsize);


    /* the ruler having been resized, recompute its height */
    RulerHeight = HeightRuler(Start, End, GapSize, postext, TickHeight,
			      PosTicks, NumGroups, NumLabels);
    /* the groups having been resized, recompute their height */
    TotalHeight = 0.0;
    for(i=0; i<NumGroups; i++)
    {
	GroupHeight[i] = HeightGroup(posblock, posrange, postext, TickHeight,
				     BlockHeight, RangeHeight, Name[i],
				     Style[i], NumLabels[i], NumNames[i],
				     PosTicks, PosBlocks, AdjustMax[i]);
	TotalHeight += (GroupHeight[i]+GapGroup);
    }


    /* draw the ruler */
    DrawRuler(xDraw, yDraw, Start, End, RealLength, Radius, TickHeight,
	      OriginAngle, GapSize, TickLines, TextLength, TextHeight,
	      postext, NumGroups, NumLabels, &From[0][0], PosTicks, 1);

    /* draw the groups */
    for(i=0; i<NumGroups; i++)
    {
	Radius-=( GroupHeight[i]+GapGroup );
	DrawGroup(xDraw, yDraw, posblock, posrange, postext, TickHeight,
		  BlockHeight, RangeHeight, RealLength, TextLength,
		  TextHeight, Radius, RadiusMax, From[i], To[i], Name[i],
		  FromSymbol[i], ToSymbol[i], Style[i], InterSymbol,
		  InterTicks, NumLabels[i], GroupName[i], OriginAngle,
		  NumNames[i], PosTicks, PosBlocks, Adjust[i], InterColor,
		  Color[i]);
	ajStrDel(&GroupName[i]);
    }


    /* close the input file */
    ajFileClose(&infile);
    ajStrDel(&line);

    /* close the graphical window */
    ajGraphCloseWin();

    ajExit();
    return 0;
}







/* compute the character size that fits all elements of the ruler provided that
   the height and the length of all strings are at most TextHeight and TextLength, respectively */
float TextRuler(float Start, float End, ajint GapSize, float TextLength,
		float TextHeight, AjPStr PosTicks, ajint NumGroups,
		ajint *NumLabels)
{
    ajint i;
    ajint j;
    AjPStr token;
    AjPStr string = ajStrNew();
    float charsize;
    float minsize = 100.0;

    ajStrFromInt(&string, Start);
    charsize = ajGraphFitTextOnLine( 0, 0, TextLength, TextLength,
				    ajStrStr(string), TextHeight );
    if( charsize < minsize )
	minsize = charsize;

    for(i=GapSize; i<End; i+=GapSize) 
	if( i>Start ) 
	{
	    ajStrFromInt(&string, i);
	    charsize = ajGraphFitTextOnLine( 0, 0, TextLength, TextLength,
					    ajStrStr(string), TextHeight );
	    if( charsize < minsize )
		minsize = charsize;
	}

    for(i=0; i<NumGroups; i++)
    {
	for(j=0; j<NumLabels[i]; j++)
	{
	    if( ajStrMatchCaseC(Style[i][j], "Tick") &&
	       ajStrMatchCaseC(PosTicks, "Out") )
	    {
		token = ajStrTokC(Name[i][j], ";");
		charsize = ajGraphFitTextOnLine( 0, 0, TextLength,
						TextLength, ajStrStr(token),
						TextHeight );
		if( charsize < minsize ) minsize = charsize;
	    }
	}
    }

    ajStrDel(&string);

    return minsize;
}







/*
 *  compute the character size that fits all elements of the ruler provided
 *  that the height and the length of all strings are multiplied by TextCoef
 */
float TextRulerStr(float Start, float End, ajint GapSize, float TextCoef,
		   AjPStr PosTicks, ajint NumGroups, ajint *NumLabels)
{
    ajint i;
    ajint  j;
    AjPStr token;
    AjPStr string = ajStrNew();
    float charsize;
    float minsize = 100.0;
    float stringLength;
    float stringHeight;

    ajStrFromInt(&string, Start);
    stringLength = ajGraphTextLength( 0, 0, 1, 1, ajStrStr(string) );
    stringHeight = ajGraphTextHeight(0, 0, 1, 1);
    charsize = ajGraphFitTextOnLine( 0, 0, stringLength/TextCoef,
				    stringLength/TextCoef, ajStrStr(string),
				    stringHeight/TextCoef );
    if( charsize < minsize ) minsize = charsize;

    for(i=GapSize; i<End; i+=GapSize)
	if( i>Start )
	{
	    ajStrFromInt(&string, i);
	    stringLength = ajGraphTextLength( 0, 0, 1, 1, ajStrStr(string) );
	    stringHeight = ajGraphTextHeight(0, 0, 1, 1);
	    charsize = ajGraphFitTextOnLine( 0, 0, stringLength/TextCoef, stringLength/TextCoef, ajStrStr(string), stringHeight/TextCoef );
	    if( charsize < minsize )
		minsize = charsize;
	}
    
    for(i=0; i<NumGroups; i++)
	for(j=0; j<NumLabels[i]; j++)
	{
	    if( ajStrMatchCaseC(Style[i][j], "Tick") &&
	       ajStrMatchCaseC(PosTicks, "Out") ) {
		token = ajStrTokC(Name[i][j], ";");
		stringLength = ajGraphTextLength( 0, 0, 1, 1,
						 ajStrStr(token) );
		stringHeight = ajGraphTextHeight(0, 0, 1, 1);
		charsize = ajGraphFitTextOnLine( 0, 0, stringLength/TextCoef,
						stringLength/TextCoef,
						ajStrStr(token),
						stringHeight/TextCoef );
		if( charsize < minsize )
		    minsize = charsize;
	    }
	}
    
    ajStrDel(&string);

    return minsize;
}






/* compute the ruler's height */
float HeightRuler(float Start, float End, ajint GapSize, float postext,
		  float TickHeight, AjPStr PosTicks, ajint NumGroups,
		  ajint *NumLabels)
{
    ajint i;
    ajint j;
    float RulerHeight;
    float stringLength;
    float maxLength = 0.0;
    AjPStr token;
    AjPStr string = ajStrNew();

    RulerHeight = TickHeight+postext;

    ajStrFromInt(&string, Start);
    stringLength = ajGraphTextLength( 0, 0, 1, 0, ajStrStr(string) );
    if( stringLength>maxLength ) maxLength = stringLength;
    for(i=GapSize; i<End; i+=GapSize)
	if( i>Start )
	{
	    ajStrFromInt(&string, i);
	    stringLength = ajGraphTextLength( 0, 0, 1, 0, ajStrStr(string) );
	    if( stringLength>maxLength )
		maxLength = stringLength;
	}

    for(i=0; i<NumGroups; i++)
    {
	for(j=0; j<NumLabels[i]; j++)
	{
	    if( ajStrMatchCaseC(Style[i][j], "Tick") &&
	       ajStrMatchCaseC(PosTicks, "Out") )
	    {
		token = ajStrTokC(Name[i][j], ";");
		stringLength = ajGraphTextLength( 0, 0, 1, 0,
						 ajStrStr(token) );
		if( stringLength>maxLength )
		    maxLength = stringLength;
	    }
	}
    }

    RulerHeight += maxLength;

    ajStrDel(&string);

    return RulerHeight;
}






/* draw a ruler */
void DrawRuler(float xDraw, float yDraw, float Start, float End,
	       float RealLength, float Radius, float TickHeight,
	       float OriginAngle, ajint GapSize, AjPStr TickLines,
	       float TextLength, float TextHeight, float postext,
	       ajint NumGroups, ajint *NumLabels, float *From, AjPStr PosTicks,
	       ajint Color)
{
    ajint i;
    AjPStr string = ajStrNew();
    AjPStr posticks = ajStrNew();
    float *xy;
    float Angle;

    ajGraphSetFore(Color);

    ajGraphCircle( xDraw, yDraw, Radius );

    xy = (float *)AJALLOC( 2*sizeof(float) );
    ajStrSetC(&posticks, "Out");

    /* set the circle's origin */
    ajStrFromInt(&string, Start);
    if( ajStrCmpCaseCC(ajStrStr(TickLines), "Y")==0 )
    {
	Angle = ComputeAngle(RealLength, 0, OriginAngle);
	xy = ajComputeCoord(xDraw, yDraw, Radius, Angle);
	ajGraphDrawLine(xDraw, yDraw, xy[0], xy[1]);
    }
    if( !OverlapTickRuler(NumGroups, NumLabels, From, PosTicks, Start) )
	DrawTicks(xDraw, yDraw, RealLength, Radius, TickHeight, 0,
		  string, OriginAngle, TextLength, TextHeight, postext,
		  posticks, 1, 0, Color);

    /* draw the ruler's ticks */
    for(i=GapSize; i<End; i+=GapSize) if( i>Start )
    {
	ajStrFromInt(&string, i);
	if( ajStrCmpCaseCC(ajStrStr(TickLines), "Y")==0 )
	{
	    Angle = ComputeAngle(RealLength, i-Start, OriginAngle);
	    xy = ajComputeCoord(xDraw, yDraw, Radius, Angle);
	    ajGraphDrawLine(xDraw, yDraw, xy[0], xy[1]);
	}
	if( !OverlapTickRuler(NumGroups, NumLabels, From, PosTicks, i) )
	    DrawTicks(xDraw, yDraw, RealLength, Radius, TickHeight,
		      i-Start, string, OriginAngle, TextLength,
		      TextHeight, postext, posticks, 1, 0, Color);
    }

    ajStrDel(&string);
    return;
}






/* draw a Tick */
void DrawTicks(float xDraw, float yDraw, float RealLength, float Radius,
	       float TickHeight, float From, AjPStr Name2, float OriginAngle,
	       float TextLength, float TextHeight, float postext,
	       AjPStr PosTicks, ajint NumNames, ajint Adjust, ajint Color)
{
    float Angle;
    float StartAngle;
    float EndAngle;
    float *xy1;
    float *xy2;
    float *xy3;
    float stringLength;
    float r1Ticks = Radius;
    float r2Ticks = r1Ticks+TickHeight;
    AjPStr token;

    ajGraphSetFore(Color);

    Angle = ComputeAngle(RealLength, From, OriginAngle);

    xy1 = (float *)AJALLOC( 2*sizeof(float) );
    xy2 = (float *)AJALLOC( 2*sizeof(float) );
    xy3 = (float *)AJALLOC( 2*sizeof(float) );

    xy1 = ajComputeCoord(xDraw, yDraw, r1Ticks, Angle);
    xy2 = ajComputeCoord(xDraw, yDraw, r2Ticks, Angle);
    ajGraphDrawLine( xy1[0], xy1[1], xy2[0], xy2[1] );

    if( ajStrMatchCaseC(PosTicks, "In") )
    {
	stringLength = HorTextPileLengthMax(Name2, NumNames);
	StartAngle = ComputeAngle(RealLength, From+stringLength/2,
				  OriginAngle);
	EndAngle = ComputeAngle(RealLength, From-stringLength/2, OriginAngle);
	HorTextPile(xDraw, yDraw, r2Ticks+(Adjust*postext), StartAngle,
		    EndAngle, Name2, postext, 1);
    }
    else
    {
	token = ajStrTokC(Name2, ";");
	/*ajStrSubstituteCC(&Name2, ";", " ");*/
	stringLength = ajGraphTextLength( 0, 0, 1, 1, ajStrStr(token) );
	xy1 = ajComputeCoord(xDraw, yDraw, r2Ticks+postext, Angle);
	xy2 = ajComputeCoord(xDraw, yDraw, r2Ticks+postext+stringLength,
			     Angle);
	xy3 = ajComputeCoord(xDraw, yDraw, r2Ticks+postext-stringLength,
			     Angle);
	if( (Angle>=0.0 && Angle<=90.0) || (Angle>=270.0 && Angle<=360.0) ||
	   (Angle>=360.0 && Angle<=450.0) || (Angle>=630.0 && Angle<=720.0) )
	    ajGraphDrawTextOnLine( xy1[0], xy1[1], xy2[0], xy2[1],
				  ajStrStr(token), 0.0 );
	else
	    ajGraphDrawTextOnLine( xy1[0], xy1[1], xy3[0], xy3[1],
				  ajStrStr(token), 1.0 );
    }

    return;
}






/* draw a Block */
void DrawBlocks(float xDraw, float yDraw, float RealLength, float Radius,
		ajint BlockHeight, float From, float To, AjPStr Name2,
		float postext, float OriginAngle, AjPStr PosBlocks,
		ajint NumNames, ajint Adjust, ajint Color)
{
    float StartAngle;
    float EndAngle;
    float stringLength;
    float stringHeight;
    float r1Blocks = Radius+(1.0*BlockHeight/2);
    float r2Blocks = r1Blocks-BlockHeight;

    ajGraphSetFore(Color);

    StartAngle = ComputeAngle(RealLength, From, OriginAngle);
    EndAngle = ComputeAngle(RealLength, To, OriginAngle);
    ajGraphRectangleOnCurve(xDraw, yDraw, r2Blocks, BlockHeight,
			    StartAngle, EndAngle);

    stringLength = HorTextPileLengthMax(Name2, NumNames);
    stringHeight = ajGraphTextHeight(0, 0, 1, 1);
    StartAngle = ComputeAngle(RealLength, (To+From)/2+stringLength/2,
			      OriginAngle);
    EndAngle = ComputeAngle(RealLength, (To+From)/2-stringLength/2,
			    OriginAngle);
    if( ajStrMatchCaseC(PosBlocks, "Out") )
	HorTextPile(xDraw, yDraw, r1Blocks+(Adjust*postext), StartAngle,
		    EndAngle, Name2, postext, 1);
    else
	HorTextPile(xDraw, yDraw,
		    (r1Blocks+r2Blocks)/2-(stringHeight/2)-postext,
		    StartAngle, EndAngle, Name2, postext, 1);

    return;
}






/* draw a Range */
void DrawRanges(float xDraw, float yDraw, float RealLength, float Radius,
		float RangeHeight, float From, float To, char FromSymbol,
		char ToSymbol, AjPStr Name2, float OriginAngle,
		ajint NumNames, float postext, ajint Adjust, ajint Color)
{
    float StartAngle, EndAngle;
    float stringLength;
    float rRanges = Radius;
    float rupper = rRanges+(1.0*RangeHeight/2);
    float BoundaryLength;

    ajGraphSetFore(Color);

    StartAngle = ComputeAngle(RealLength, From, OriginAngle);
    EndAngle = ComputeAngle(RealLength, To, OriginAngle);
    ajGraphPartCircle(xDraw, yDraw, rRanges, StartAngle, EndAngle);

    if( RangeHeight>(From-To)/3 )
	BoundaryLength = (From-To)/3;
    else
	BoundaryLength = RangeHeight;

    if( FromSymbol=='<' )
	DrawArrowHeadsOnCurve(xDraw, yDraw, RealLength, RangeHeight,
			      BoundaryLength, rRanges, StartAngle,
			      OriginAngle, +1);
    if( FromSymbol=='>' )
	DrawArrowHeadsOnCurve(xDraw, yDraw, RealLength, RangeHeight,
			      BoundaryLength, rRanges, StartAngle,
			      OriginAngle, -1);
    if( FromSymbol=='[' )
	DrawBracketsOnCurve(xDraw, yDraw, RealLength, RangeHeight,
			    BoundaryLength, rRanges, StartAngle,
			    OriginAngle, +1);
    if( FromSymbol==']' )
	DrawBracketsOnCurve(xDraw, yDraw, RealLength, RangeHeight,
			    BoundaryLength, rRanges, StartAngle,
			    OriginAngle, -1);

    if( ToSymbol=='<' )
	DrawArrowHeadsOnCurve(xDraw, yDraw, RealLength, RangeHeight,
			      BoundaryLength, rRanges, EndAngle,
			      OriginAngle, +1);
    if( ToSymbol=='>' )
	DrawArrowHeadsOnCurve(xDraw, yDraw, RealLength, RangeHeight,
			      BoundaryLength, rRanges, EndAngle,
			      OriginAngle, -1);
    if( ToSymbol=='[' )
	DrawBracketsOnCurve(xDraw, yDraw, RealLength, RangeHeight,
			    BoundaryLength, rRanges, EndAngle,
			    OriginAngle, +1);
    if( ToSymbol==']' )
	DrawBracketsOnCurve(xDraw, yDraw, RealLength, RangeHeight,
			    BoundaryLength, rRanges, EndAngle,
			    OriginAngle, -1);

    stringLength = HorTextPileLengthMax(Name2, NumNames);
    StartAngle = ComputeAngle(RealLength, (To+From)/2+stringLength/2,
			      OriginAngle);
    EndAngle = ComputeAngle(RealLength, (To+From)/2-stringLength/2,
			    OriginAngle);
    HorTextPile(xDraw, yDraw, rupper+(Adjust*postext), StartAngle,
		EndAngle, Name2, postext, 1);

    return;
}






/* draw an InterBlock */
void InterBlocks(float xDraw, float yDraw, float RealLength, float Radius,
		 float BlockHeight, float From, float To, float OriginAngle,
		 AjPStr InterSymbol, ajint Color)
{
    float StartAngle;
    float  EndAngle;
    float r1Inter = Radius+(1.0*BlockHeight/2);
    float r2Inter = r1Inter-BlockHeight;

    ajGraphSetFore(Color);

    StartAngle = ComputeAngle(RealLength, To, OriginAngle);
    EndAngle = ComputeAngle(RealLength, From, OriginAngle);

    if( ajStrCmpCaseCC(ajStrStr(InterSymbol), "Y")==0 )
	ajGraphPartCircle(xDraw, yDraw, (r1Inter+r2Inter)/2, StartAngle,
			  EndAngle);
}







/* compute the angle knowing the length */
float ComputeAngle(float RealLength, float Length, float OriginAngle)
{
    float i;
    float j;

    i = Length/RealLength;
    j = i * 360 - OriginAngle;

    return 360-j;
}







/* draw arrowheads on a curve*/
void DrawArrowHeadsOnCurve(float xDraw, float yDraw, float RealLength,
			   float Height, float Length, float Radius,
			   float Angle, float OriginAngle, ajint Way)
{
    float *xy1;
    float *xy2;
    float StartAngle;
    float EndAngle;
    float pos;
    float middle = 1.0*Height/2;

    xy1 = (float *)AJALLOC( 2*sizeof(float) );
    xy2 = (float *)AJALLOC( 2*sizeof(float) );

    StartAngle = ComputeAngle(RealLength, 0, OriginAngle);
    EndAngle = ComputeAngle(RealLength, Length, OriginAngle);
    pos = EndAngle-StartAngle;

    if(Way==1)
    {
	xy1 = ajComputeCoord(xDraw, yDraw, Radius, Angle);
	xy2 = ajComputeCoord(xDraw, yDraw, Radius+middle, Angle+pos);
	ajGraphDrawLine( xy1[0], xy1[1], xy2[0], xy2[1] );
	xy2 = ajComputeCoord(xDraw, yDraw, Radius-middle, Angle+pos);
	ajGraphDrawLine( xy1[0], xy1[1], xy2[0], xy2[1] );
    }
    if(Way==-1)
    {
	xy1 = ajComputeCoord(xDraw, yDraw, Radius, Angle);
	xy2 = ajComputeCoord(xDraw, yDraw, Radius+middle, Angle-pos);
	ajGraphDrawLine( xy1[0], xy1[1], xy2[0], xy2[1] );
	xy2 = ajComputeCoord(xDraw, yDraw, Radius-middle, Angle-pos);
	ajGraphDrawLine( xy1[0], xy1[1], xy2[0], xy2[1] );
    }

    return;
}







/* draw brackets on a curve*/
void DrawBracketsOnCurve(float xDraw, float yDraw, float RealLength,
			 float Height, float Length, float Radius,
			 float Angle, float OriginAngle, ajint Way)
{
    float *xy1;
    float *xy2;
    float StartAngle;
    float EndAngle;
    float pos;
    float middle = 1.0*Height/2;

    xy1 = (float *)AJALLOC( 2*sizeof(float) );
    xy2 = (float *)AJALLOC( 2*sizeof(float) );

    StartAngle = ComputeAngle(RealLength, 0, OriginAngle);
    EndAngle = ComputeAngle(RealLength, Length, OriginAngle);
    pos = EndAngle-StartAngle;

    if(Way==1)
    {
	xy1 = ajComputeCoord(xDraw, yDraw, Radius+middle, Angle);
	xy2 = ajComputeCoord(xDraw, yDraw, Radius-middle, Angle);
	ajGraphDrawLine( xy1[0], xy1[1], xy2[0], xy2[1] );
	ajGraphPartCircle(xDraw, yDraw, Radius+middle, Angle, Angle+pos);
	ajGraphPartCircle(xDraw, yDraw, Radius-middle, Angle, Angle+pos);
    }
    if(Way==-1)
    {
	xy1 = ajComputeCoord(xDraw, yDraw, Radius+middle, Angle);
	xy2 = ajComputeCoord(xDraw, yDraw, Radius-middle, Angle);
	ajGraphDrawLine( xy1[0], xy1[1], xy2[0], xy2[1] );
	ajGraphPartCircle(xDraw, yDraw, Radius+middle, Angle, Angle-pos);
	ajGraphPartCircle(xDraw, yDraw, Radius-middle, Angle, Angle-pos);
    }

    return;
}







/* write a pile of horizontal text strings */
void HorTextPile(float x, float y, float Radius, float StartAngle,
		 float EndAngle, AjPStr Name2, float postext,
		 ajint NumNames)
{
    float rupper;
    float stringHeight;
    float totalHeight;
    AjPStr token;
    ajint i;

    totalHeight = Radius+postext;
    for(i=0; i<NumNames; i++)
    {
	if(i==0)
	    token = ajStrTokC(Name2, ";");
	else
	    token = ajStrTokC(NULL, ";");
	stringHeight = ajGraphTextHeight(0, 0, 1, 1);
	rupper = totalHeight+stringHeight;
	if(token && ajStrStr(token))
	    ajGraphDrawTextOnCurve(x, y, (totalHeight+rupper)/2, StartAngle,
				   EndAngle, ajStrStr(token), 0.5);
	totalHeight+=(stringHeight+postext);
    }

    return;
}






/* compute the height of a pile of horizontal text strings */
float HorTextPileHeight(float postext, ajint NumNames)
{
    float stringHeight;
    float totalHeight;
    ajint i;

    totalHeight = 0.0;
    for(i=0; i<NumNames; i++)
    {
	stringHeight = ajGraphTextHeight(0, 0, 1, 1);
	totalHeight+=(stringHeight+postext);
    }

    return totalHeight;
}






/*
 *  compute the maximum length of a pile of horizontal text strings 
 *  (this is the length of the longest string)
 */
float HorTextPileLengthMax(AjPStr Name2, ajint NumNames)
{
    float stringLength;
    float maxLength;
    ajint i;
    AjPStr token;

    maxLength = 0.0;
    for(i=0; i<NumNames; i++)
    {
	if(i==0) token = ajStrTokC(Name2, ";");
	else token = ajStrTokC(NULL, ";");
	stringLength = ajGraphTextLength( 0, 0, 1, 1, ajStrStr(token) );
	if( stringLength>maxLength )
	    maxLength = stringLength;
    }

    return maxLength;
}







/* read the beginning of the input file */
void ReadInput(AjPFile infile, float *Start, float *End)
{
    AjPStr line;

    line = ajStrNew();
    while( ajFileReadLine(infile, &line) )
    {
	/* read the start and end positions */
	if( ajStrPrefixC(line, "Start") )
	    sscanf(ajStrStr(line), "%*s%f", Start);
	if( ajStrPrefixC(line, "End") )
	    sscanf(ajStrStr(line), "%*s%f", End);
    }

    ajStrDel(&line);
    return;
}






/* read a group */
AjPStr ReadGroup(AjPFile infile, float *From, float *To, AjPStr *Name,
		 char *FromSymbol, char *ToSymbol, AjPStr *Style2,
		 ajint *NumLabels, ajint *NumNames, ajint *Color)
{
    ajint i;
    ajint j;
    AjPStr GroupName;
    AjPStr line;
    AjPStr token;
    char *style;
    ajlong pos;

    line = ajStrNew();
    GroupName = ajStrNew();
    style = (char *)AJALLOC( 10*sizeof(char) );

    /* read the group's name */
    pos = ajFileTell(infile);
    while( ajFileReadLine(infile, &GroupName) )
    {
	token = ajStrTokC(GroupName, " \n\t\r\f");
	if( ajStrLen(token)!=0 )
	{
	    if( ajStrMatchCaseC(GroupName, "label") ||
	       ajStrMatchCaseC(GroupName, "endgroup") )
		ajStrAssC(&GroupName, " ");
	    if( ajStrLen(GroupName)>20 )
		ajStrCut( &GroupName, 20, ajStrLen(GroupName) );
	    break;
	}
    }

    i = 0;
    ajFileSeek(infile, pos, 0);
    while( ajFileReadLine(infile, &line) )
    {
	token = ajStrTokC(line, " \n\t\r\f");
	if( ajStrLen(token)!=0 )
	{
	    if( ajStrPrefixC(line, "endgroup") )
		break;
	    else
	    {
		/* read the group's label(s) */
		if( ajStrPrefixC(line, "label") )
		{
		    while( ajFileReadLine(infile, &line) )
		    {
			token = ajStrTokC(line, " \n\t\r\f");
			if( ajStrLen(token)!=0 )
			{
			    FromSymbol[i] = '<';
			    ToSymbol[i] = '>';
			    sscanf( ajStrStr(line), "%s", style );
			    if( ajStrMatchCaseCC(style, "Tick") )
				sscanf( ajStrStr(line), "%*s %f %d %*c",
				       &From[i], &Color[i] );
			    if( ajStrMatchCaseCC(style, "Block") )
				sscanf( ajStrStr(line), "%*s %f %f %d %*c",
				       &To[i], &From[i], &Color[i] );
			    if( ajStrMatchCaseCC(style, "Range") )
				sscanf( ajStrStr(line), "%*s %f %f %c %c %d"
				       " %*c", &To[i], &From[i],
				       &FromSymbol[i], &ToSymbol[i],
				       &Color[i] );
			    ajStrAssC(&Style2[i], style);
			    break;
			}
		    }
	    
		    j = 0;
		    /* read the label's name(s) */
		    while( ajFileReadLine(infile, &line) )
		    {
			token = ajStrTokC(line, " \n\t\r\f");
			if( ajStrLen(token)!=0 )
			{
			    if( ajStrPrefixC(line, "endlabel") )
				break;
			    else
			    {
				ajStrApp(&Name[i], line);
				ajStrAppC(&Name[i], ";");
				j++;
			    }
			}
		    }
		    NumNames[i] = j;
		    i++;
		}
	    }
	}
    }
    *NumLabels = i;

    ajStrDel(&line);
    return GroupName;
}







/*
 *  compute the character size that fits all elements of a group
 *  provided that the height and the length of all strings are at most
 *  TextHeight and TextLength, respectively
 */
float TextGroup(float TextHeight, float TextLength, AjPStr *Name,
		ajint NumLabels, ajint *NumNames, AjPStr GroupName,
		AjPStr *Style2, float *From, float *To, float BlockHeight,
		AjPStr PosTicks)
{
    ajint i;
    ajint j;
    float charsize;
    float  minsize = 100.0;
    AjPStr token;

    for(i=0; i<NumLabels; i++)
    {
	for(j=0; j<NumNames[i]; j++)
	{
	    if( !(ajStrMatchCaseC(Style2[i], "Tick") &&
		  ajStrMatchCaseC(PosTicks, "Out")) )
	    {
		if(j==0)
		    token = ajStrTokC(Name[i], ";");
		else
		    token = ajStrTokC(NULL, ";");
		if( ajStrMatchCaseC(Style2[i], "Block") &&
		   ((From[i]-To[i])<TextLength) )
		    charsize = ajGraphFitTextOnLine( 0, 0, From[i]-To[i],
						    From[i]-To[i],
						    ajStrStr(token),
						    TextHeight );
		else
		    charsize = ajGraphFitTextOnLine( 0, 0, TextLength,
						    TextLength,
						    ajStrStr(token),
						    TextHeight );
		if( charsize < minsize )
		    minsize = charsize;
	    }
	}
    }
    
    return minsize;
}






/* 
 *  compute the character size that fits all elements of a group provided that
 *  the height and the length of all strings are multiplied by TextCoef
 */
float TextGroupStr(AjPStr *Name2, ajint NumLabels, ajint *NumNames,
		   AjPStr GroupName, float TextCoef, AjPStr *Style2,
		   float *From, float *To, float BlockHeight, AjPStr PosTicks)
{
    ajint i;
    ajint j;
    float charsize;
    float minsize = 100.0;
    float stringLength;
    float stringHeight;
    AjPStr token;

    for(i=0; i<NumLabels; i++)
    {
	for(j=0; j<NumNames[i]; j++)
	{
	    if( !(ajStrMatchCaseC(Style2[i], "Tick") &&
		  ajStrMatchCaseC(PosTicks, "Out")) )
	    {
		if(j==0)
		    token = ajStrTokC(Name2[i], ";");
		else
		    token = ajStrTokC(NULL, ";");
		stringLength = ajGraphTextLength( 0, 0, 1, 1,
						 ajStrStr(token) );
		stringHeight = ajGraphTextHeight(0, 0, 1, 1);
		if( ajStrMatchCaseC(Style2[i], "Block") &&
		   ((From[i]-To[i])<stringLength) )
		    charsize = ajGraphFitTextOnLine( 0, 0,
						    (From[i]-To[i])/TextCoef,
						    (From[i]-To[i])/TextCoef,
						    ajStrStr(token),
						    stringHeight/TextCoef );
		else
		    charsize = ajGraphFitTextOnLine( 0, 0,
						    stringLength/TextCoef,
						    stringLength/TextCoef,
						    ajStrStr(token),
						    stringHeight/TextCoef );
		if( charsize < minsize )
		    minsize = charsize;
	    }
	}
    }

    return minsize;
}






/* compute the height of a group depending on what's in it */
float HeightGroup(float posblock, float posrange, float postext,
		  float TickHeight, float BlockHeight, float RangeHeight,
		  AjPStr *Name2, AjPStr *Style2, ajint NumLabels,
		  ajint *NumNames, AjPStr PosTicks, AjPStr PosBlocks,
		  ajint Adjust)
{
    ajint i;
    float GroupHeight;
    float uheight;
    float umaxheight = 0.0;
    float lheight;
    float lmaxheight = 0.0;


    for(i=0; i<NumLabels; i++)
    {
	if( ajStrMatchCaseC(Style2[i], "Tick") &&
	   ajStrMatchCaseC(PosTicks, "In") )
	{
	    uheight = TickHeight;
	    lheight = 0.0;
	    uheight+=HorTextPileHeight(postext, 1);
	    if( uheight > umaxheight )
		umaxheight = uheight;
	    if( lheight > lmaxheight )
		lmaxheight = lheight;
	}

	if( ajStrMatchCaseC(Style2[i], "Block") )
	{
	    uheight = 1.0*BlockHeight/2;
	    lheight = 1.0*BlockHeight/2;
	    if( ajStrMatchCaseC(PosBlocks, "Out") )
		uheight+=HorTextPileHeight(postext, 1);
	    if( uheight > umaxheight )
		umaxheight = uheight;
	    if( lheight > lmaxheight )
		lmaxheight = lheight;
	}

	if( ajStrMatchCaseC(Style2[i], "Range") )
	{
	    uheight = 1.0*RangeHeight/2;
	    lheight = 1.0*RangeHeight/2;
	    uheight+=HorTextPileHeight(postext, 1);
	    if( uheight > umaxheight )
		umaxheight = uheight;
	    if( lheight > lmaxheight )
		lmaxheight = lheight;
	}
    }

    GroupHeight = umaxheight+lmaxheight+(Adjust*postext);

    return GroupHeight;
}






/* find whether horizontal text strings overlap within a group */
ajint OverlapTextGroup(AjPStr *Name2, AjPStr *Style2, ajint NumLabels,
		     float *From, float *To, float Start, float End,
		     AjPStr PosTicks, ajint *Adjust)
{
    ajint i;
    ajint j;
    ajint AdjustMax;
    AjPStr token;
    float FromText[MAXLABELS];
    float ToText[MAXLABELS];
    float llim;
    float ulim;
    float stringLength;
    
    
    /* compute the length of the horizontal strings */
    for(i=0; i<NumLabels; i++)
    {
	if( ajStrMatchCaseC(Style2[i], "Tick") &&
	   ajStrMatchCaseC(PosTicks, "In") )
	{
	    token = ajStrTokC(Name2[i], ";");
	    stringLength = ajGraphTextLength( 0, 0, 1, 1, ajStrStr(token) );
	    ulim = From[i]+stringLength/2;
	    if( ulim>(End-Start-1) )
		ulim = End-Start-1-ulim;
	    llim = From[i]-stringLength/2;
	    if( (ulim<0.0) || (llim<0.0) )
	    {
		FromText[i] = llim;
		ToText[i] = ulim;
	    }
	    else
	    {
		FromText[i] = ulim;
		ToText[i] = llim;
	    }
	}
	else if( ajStrMatchCaseC(Style2[i], "Block") )
	{
	    token = ajStrTokC(Name2[i], ";");
	    stringLength = ajGraphTextLength( 0, 0, 1, 1, ajStrStr(token) );
	    ulim = (To[i]+From[i])/2+stringLength/2;
	    if( ulim>(End-Start-1) )
		ulim = End-Start-1-ulim;
	    llim = (To[i]+From[i])/2-stringLength/2;
	    if( (ulim<0.0) || (llim<0.0) )
	    {
		FromText[i] = llim;
		ToText[i] = ulim;
	    }
	    else
	    {
		FromText[i] = ulim;
		ToText[i] = llim;
	    }
	}
	else if( ajStrMatchCaseC(Style2[i], "Range") )
	{
	    token = ajStrTokC(Name2[i], ";");
	    stringLength = ajGraphTextLength( 0, 0, 1, 1, ajStrStr(token) );
	    ulim = (To[i]+From[i])/2+stringLength/2;
	    if( ulim>(End-Start-1) )
		ulim = End-Start-1-ulim;
	    llim = (To[i]+From[i])/2-stringLength/2;
	    if( (ulim<0.0) || (llim<0.0) )
	    {
		FromText[i] = llim;
		ToText[i] = ulim;
	    }
	    else
	    {
		FromText[i] = ulim;
		ToText[i] = llim;
	    }
	}
	else
	{
	    FromText[i] =  ToText[i] = 0.0;
	    /*    ajUser("HELLO should not get here: *%d*%S*",i,Style2[i]);*/
	}
    }
 
    /*
     *  if some strings overlap, the position of the overlapping strings
     *  is moved upwards by Adjust
     */
    for(i=0; i<NumLabels; i++)
	Adjust[i] = 0;
    for(i=0; i<NumLabels; i++)
    {
	for(j=0; j<NumLabels; j++)
	{
	    if( (i!=j) && (Adjust[i]==Adjust[j]) )
	    {
		if(j>i)
		{
		    if( FromText[i]<0.0 )
		    {
			ulim = End-Start-1+FromText[i];
			if( (ToText[j]<=ulim) && (FromText[j]>=ulim) )
			    Adjust[j] = Adjust[i]+1;
		    }
		    if( (ToText[j]<=FromText[i]) &&
		       (FromText[j]>=FromText[i]) )
			Adjust[j] = Adjust[i]+1;
		}
		if(i>j)
		{
		    if( FromText[j]<0.0 )
		    {
			ulim = End-Start-1+FromText[j];
			if( (ToText[i]<=ulim) && (FromText[i]>=ulim) )
			    Adjust[i] = Adjust[j]+1;
		    }
		    if( (ToText[i]<=FromText[j]) &&
		       (FromText[i]>=FromText[j]) )
			Adjust[i] = Adjust[j]+1;
		}
	    }
	}
    }
    
    AdjustMax = 0.0;
    for(i=0; i<NumLabels; i++)
	if( Adjust[i]>AdjustMax )
	    AdjustMax = Adjust[i];
    
    return AdjustMax;
}






/* find whether group ticks and ruler's ticks overlap */
AjBool OverlapTickRuler(ajint NumGroups, ajint *NumLabels, float *From,
			AjPStr PosTicks, ajint RulerTick)
{
    ajint i;
    ajint j;
    ajint overlap = 0;

    for(i=0; i<NumGroups; i++)
	for(j=0; j<NumLabels[i]; j++)
	{
	    if( ajStrMatchCaseC(Style[i][j], "Tick") &&
	       ajStrMatchCaseC(PosTicks, "Out") &&
	       From[(i*MAXLABELS)+j]==RulerTick )
	    {
		overlap = 1;
		break;
	    }
	}

    if( overlap==0 )
	return ajFalse;
    return ajTrue;
}






/* draw a group */
void DrawGroup(float xDraw, float yDraw, float posblock, float posrange,
	       float postext, float TickHeight, float BlockHeight,
	       float RangeHeight, float RealLength, float TextLength,
	       float TextHeight, float Radius, float RadiusMax,
	       float *From, float *To, AjPStr *Name2, char *FromSymbol,
	       char *ToSymbol, AjPStr *Style2, AjPStr InterSymbol,
	       AjPStr InterTicks, ajint NumLabels, AjPStr GroupName,
	       float OriginAngle, ajint *NumNames, AjPStr PosTicks,
	       AjPStr PosBlocks, ajint *Adjust, ajint InterColor, ajint *Color)
{
    ajint i;
    ajint j;
    ajint  NumBlocks;
    ajint Inter[MAXLABELS];


    /* draw all labels */
    for(j=0, i=0; i<NumLabels; i++)
    {
	if( ajStrMatchCaseC(Style2[i], "Tick") )
	{
	    if( ajStrMatchCaseC(PosTicks, "In") )
	    {
		DrawTicks(xDraw, yDraw, RealLength, Radius, TickHeight,
			  From[i], Name2[i], OriginAngle, TextLength,
			  TextHeight, postext, PosTicks, NumNames[i],
			  Adjust[i], Color[i]);
		if( ajStrCmpCaseCC(ajStrStr(InterTicks), "Y")==0 )
		    ajGraphCircle( xDraw, yDraw, Radius );
	    }
	    else
		DrawTicks(xDraw, yDraw, RealLength, RadiusMax, TickHeight,
			  From[i], Name2[i], OriginAngle, TextLength,
			  TextHeight, postext, PosTicks, NumNames[i],
			  Adjust[i], Color[i]);
	}

	if( ajStrMatchCaseC(Style2[i], "Block") )
	{
	    DrawBlocks(xDraw, yDraw, RealLength, Radius-posblock,
		       BlockHeight, From[i], To[i], Name2[i], postext,
		       OriginAngle, PosBlocks, NumNames[i], Adjust[i],
		       Color[i]);
	    Inter[j++] = i;
	}

	if( ajStrMatchCaseC(Style2[i], "Range") )
	    DrawRanges(xDraw, yDraw, RealLength, Radius-posrange,
		       RangeHeight, From[i], To[i], FromSymbol[i],
		       ToSymbol[i], Name2[i], OriginAngle, NumNames[i],
		       postext, Adjust[i], Color[i]);
    }
    NumBlocks = j;

    /* draw all interblocks */
    for(i=0; i<NumBlocks-1; i++)
	InterBlocks(xDraw, yDraw, RealLength, Radius-posblock, BlockHeight,
		    From[Inter[i]], To[Inter[i+1]], OriginAngle,
		    InterSymbol, InterColor);

    return;
}
